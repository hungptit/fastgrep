#include "clara.hpp"
#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include "params.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/regex_matchers.hpp"
#include <algorithm>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

/**
 * The grep execution process has two steps:
 * 1. Expand the search paths and get the list of search files.
 * 2. Search for given pattern using file contents and display the search results.
 */
namespace {
    struct InputParams {
        std::string pattern;                 // Grep pattern
        std::string path_pattern;            // Search path pattern
        std::vector<std::string> paths;      // Input files and folders
        std::vector<std::string> extensions; // File extension want to search.
        fastgrep::Params parameters;         // Grep parameters
        void print() const {
            fmt::print("Pattern: {}\n", pattern);
            fmt::print("Path pattern: {}\n", pattern);
            parameters.print();
        }
    };

    // Use clara to parse input argument.
    InputParams parse_input_arguments(int argc, char *argv[]) {
        InputParams params;

        // Input argument
        bool help = false;
        bool linenum = true;        // Display line number.
        bool inverse_match = false; // Inverse match i.e display lines that do not match given pattern.
        bool exact_match = false;   // Use exact matching algorithm.
        bool ignore_case = false;   // Ignore case.
        bool use_memmap = true;     // Read the file content using memory mapped approach.
        bool use_stream = false;    // Read the file content using streaming approach.
        bool utf8 = false;          // Support UTF8.
        bool color = false;         // Display color text.
        bool stream = false;        // grep the input stream.
        bool verbose = false;       // Display verbose information.

        auto cli = clara::Help(help) |
                   clara::Opt(verbose)["-v"]["--verbose"]("Display verbose information") |
                   clara::Opt(inverse_match)["--inverse-match"](
                       "Only print out lines that do not match the search pattern.") |
                   clara::Opt(exact_match)["--exact-match"]("Use exact matching algorithms.") |
                   clara::Opt(ignore_case)["-i"]["--ignore-case"]("Ignore case") |
                   clara::Opt(use_stream)["--stream"]("Get data from the input pipe/stream.") |
                   clara::Opt(use_memmap)["--mmap"]("Get data from the input pipe/stream.") |
                   clara::Opt(color)["-c"]["--color"]("Print out color text.") |
                   clara::Opt(utf8)["--utf8"]("Support UTF8.") |

                   clara::Opt(params.pattern, "pattern")["-e"]["--pattern"]("Search pattern.") |
                   clara::Opt(params.path_pattern, "path_pattern")["-e"]["--pattern"]("Search pattern.") |

                   // Required arguments.
                   clara::Arg(params.paths, "paths")("Search paths");

        auto result = cli.parse(clara::Args(argc, argv));
        if (!result) {
            fmt::print(stderr, "Invalid option: {}\n", result.errorMessage());
            exit(EXIT_FAILURE);
        }

        // If users want to print out the help message then display the help message and exit.
        if (help) {
            std::ostringstream oss;
            oss << cli;
            fmt::print("{}", oss.str());
            exit(EXIT_SUCCESS);
        }

        // Choose read approach
        use_memmap = use_stream ? false : true;

        // Update search parameters
        params.parameters.regex_mode =
            HS_FLAG_DOTALL | HS_FLAG_SINGLEMATCH | (ignore_case ? HS_FLAG_CASELESS : 0);
        params.parameters.info = verbose * fastgrep::VERBOSE + color * fastgrep::COLOR +
                                 linenum * fastgrep::LINENUM + utf8 * fastgrep::UTF8 +
                                 use_memmap * fastgrep::USE_MEMMAP;

        // If users do not specify the search pattern then the first elements of paths is the search
        // pattern.
        if (params.pattern.empty()) {
            if (params.paths.size() < 2) {
                throw std::runtime_error(
                    "Invalid syntax. The search pattern and search paths are required.");
            }
            params.pattern = params.paths.front();
            params.paths.erase(params.paths.begin());
        }

        if (verbose) params.print();

        return params;
    }
} // namespace

template <typename T> void fgrep(const InputParams &params) {
    T grep(params.pattern, params.parameters);
    for (auto afile : params.paths) { grep(afile.data()); }
}

int main(int argc, char *argv[]) {
    constexpr int BUFFER_SIZE = 1 << 16;
    auto params = parse_input_arguments(argc, argv);

    // Search for given pattern based on input parameters
    if (params.parameters.exact_match()) {
        // using Matcher = utils::ExactMatchSSE2;
        using Matcher = utils::ExactMatchAVX2;
        if (params.parameters.use_memmap()) {
            // using Policy = typename fastgrep::MMapPolicy<Matcher>;
            // using Reader = ioutils::MMapReader<Policy>;
            // fgrep<Reader>(params);
        } else {
            // using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher>, BUFFER_SIZE>;
            // fgrep<Reader>(params);
        }
    } else {
        using Matcher = utils::hyperscan::RegexMatcher;
        if (params.parameters.use_memmap()) {
            using Policy = typename fastgrep::MMapPolicy<Matcher>;
            using Reader = ioutils::MMapReader<Policy>;
            fgrep<Reader>(params);
        } else {
            using Policy = typename fastgrep::StreamPolicy<Matcher>;
            using Reader = ioutils::FileReader<Policy, BUFFER_SIZE>;
            fgrep<Reader>(params);
        }
    }

    return EXIT_SUCCESS;
}
