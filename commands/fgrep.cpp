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
            ("pattern,p", po::value<std::string>(&params.pattern), "Search pattern")
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

        // Pre-process input arguments
        if (vm.count("help")) {
            std::cout << desc << "\n";
            std::cout << "Examples:" << "\n";
            std::cout << "    fgrep Fooo boo.txt" << "\n";
            std::cout << "    fgrep -p Fooo boo.txt" << "\n";

            exit(EXIT_SUCCESS);
        }

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

template <typename T>
void fgrep(const InputParams &params) {
    T grep(params.pattern.data());
    for (auto afile : params.files) { grep(afile.data()); }
}

int main(int argc, char *argv[]) {
    constexpr int BUFFER_SIZE = 1 << 17;
    auto params = parse_input_arguments(argc, argv);

    // Search for given pattern based on input parameters
    if (params.parameters.exact_match) {
        // using Matcher = utils::ExactMatchSSE2;
        using Matcher = utils::ExactMatchAVX2;
        if (params.parameters.use_memmap) {
            using Reader = ioutils::MMapReader<fastgrep::GrepPolicy<Matcher>>;
            fgrep<Reader>(params);
        } else {
            using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher>, BUFFER_SIZE>;
            fgrep<Reader>(params);
        }
    } else {
        using Matcher = utils::hyperscan::RegexMatcher;
        if (params.parameters.use_memmap) {
            using Reader = ioutils::MMapReader<fastgrep::GrepPolicy<Matcher>>;
            fgrep<Reader>(params);
        } else {
            using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher>, BUFFER_SIZE>;
            fgrep<Reader>(params);
        }

    }

    return EXIT_SUCCESS;
}
