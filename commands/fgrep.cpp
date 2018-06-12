#include "boost/program_options.hpp"
#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/regex_matchers.hpp"
#include <algorithm>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

// fmt header
#include "fmt/format.h"

// cereal headers
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/deque.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

namespace {
    struct GrepParams {
        bool ignore_case;      // Ignore case distinctions
        bool invert_match;     // Select non-matching lines
        bool exact_match;      // Use exact matching algorithms.
        bool use_memmap;       // Map files into memory.
        bool show_line_number; // Show line number

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(ignore_case), CEREAL_NVP(invert_match), CEREAL_NVP(exact_match),
               CEREAL_NVP(use_memmap), CEREAL_NVP(show_line_number));
        }
    };

    struct InputParams {
        std::string pattern;            // Input patterns
        std::vector<std::string> files; // Input files and folders
        std::string output_file;        // Output file
        GrepParams parameters;

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(pattern), CEREAL_NVP(files), CEREAL_NVP(output_file),
               CEREAL_NVP(parameters));
        }
    };

    InputParams parse_input_arguments(int argc, char *argv[]) {
        namespace po = boost::program_options;
        po::options_description desc("Allowed options");
        InputParams params;
        std::vector<std::string> paths;

        // clang-format off
        desc.add_options()
            ("help,h", "Print this help")
            ("verbose,v", "Display verbose information.")
            ("exact-match", "Do not use regex engine for pattern matching.")
            ("pattern, p", po::value<std::string>(&params.pattern), "Search pattern")
            ("files,f", po::value<std::vector<std::string>>(&paths), "A list of files and folders")
            ("output-file,o", po::value<std::string>(&params.output_file), "Output file")
            ("ignore-case", po::value<bool>(&params.parameters.ignore_case)->default_value(false), "Ignore case.")
            ("invert-match", po::value<bool>(&params.parameters.invert_match)->default_value(false), "Select non-matching lines.")
            ("exact-match", po::value<bool>(&params.parameters.exact_match)->default_value(false), "Use exact matching algorithms")
            ("show-line-number", po::value<bool>(&params.parameters.show_line_number)->default_value(false), "Show line number")
            ("use-mmap", po::value<bool>(&params.parameters.use_memmap)->default_value(true), "Map files into memory.");
        // clang-format on

        // Parse input arguments
        po::positional_options_description p;
        p.add("files", -1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        // Preprocess input arguments
        if (paths.empty()) {
            throw(std::runtime_error("Must provide pattern and file paths."));
        }
        auto begin = paths.cbegin();

        // If users do not specify pattern then we use the first file
        // element as a pattern to make fgrep interface consistent
        // with that of grep.
        if (params.pattern.empty()) {
            params.pattern = *begin;
            ++begin;
        }

        for (; begin != paths.end(); ++begin) {
            // TODO: FIXME
            params.files.emplace_back(*begin);
        }

        // Display input arguments in JSON format if verbose flag is on
        if (vm.count("verbose")) {
            std::stringstream ss;
            {
                cereal::JSONOutputArchive ar(ss);
                ar(cereal::make_nvp("Input arguments", params));
            }
            fmt::print("{}\n", ss.str());
        }

        return params;
    }
} // namespace

int main(int argc, char *argv[]) {
    auto params = parse_input_arguments(argc, argv);

    constexpr int BUFFER_SIZE = 1 << 17;
    // Matchers
    using Matcher1_1 = utils::experiments::ExactMatch;
    using Matcher2_1 = utils::experiments::ExactMatchSSE2;
    using Matcher3_1 = utils::experiments::ExactMatchAVX2;

    using Matcher1_2 = utils::ExactMatchSSE2;
    using Matcher2_2 = utils::ExactMatchAVX2;
    using Matcher3_2 = utils::experiments::RegexMatcher;
    using Matcher4_2 = utils::hyperscan::RegexMatcher;

    // Simple parser
    using Reader11 =
        ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher1_1>, BUFFER_SIZE>;
    using Reader12 =
        ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher2_1>, BUFFER_SIZE>;
    using Reader13 =
        ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher3_1>, BUFFER_SIZE>;

    // Improve parser
    using Reader21 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher1_2>, BUFFER_SIZE>;
    // using Reader22 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher2_2>, BUFFER_SIZE>;
    using Reader23 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher3_2>, BUFFER_SIZE>;
    using Reader24 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher4_2>, BUFFER_SIZE>;
    using Reader25 = ioutils::MMapReader<fastgrep::GrepPolicy<Matcher4_2>>;

    // Grep the content of given files.
    Reader13 grep(params.pattern.data());
    for (auto afile : params.files) { grep(afile.data()); }

    return EXIT_SUCCESS;
}
