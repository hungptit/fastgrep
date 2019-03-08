#include "clara.hpp"
#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/reader.hpp"
#include "ioutils/regex_store_policies.hpp"
#include "ioutils/search.hpp"
#include "ioutils/search_params.hpp"
#include "ioutils/simple_store_policy.hpp"
#include "ioutils/stream.hpp"
#include "params.hpp"
#include "utils/matchers.hpp"
#include "utils/regex_matchers.hpp"
#include <deque>
#include <string>
#include <vector>

/**
 * The grep execution process has two steps:
 * 1. Expand the search paths and get the list of search files.
 * 2. Search for given pattern using file contents and display the search results.
 */
namespace {
    void copyright() {
        fmt::print("{}\n", "fgrep version 0.1.0");
        fmt::print("{}\n", "Hung Dang <hungptit@gmail.com>");
    }

    struct InputParams {
        std::string pattern;            // Grep pattern
        std::string path_pattern;       // Search path pattern
        std::vector<std::string> paths; // Input files and folders
        fastgrep::Params parameters;    // Grep parameters
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
        bool linenum = false;       // Display line number.
        bool inverse_match = false; // Inverse match i.e display lines that do not match given pattern.
        bool exact_match = false;   // Use exact matching algorithm.
        bool ignore_case = false;   // Ignore case.
        bool recursive = false;     // Recursively search subdirectories.

        bool use_memmap = false; // Read the file content using memory mapped approach.

        bool stdin = false;   // Read data from STDIN.
        bool color = false;   // Display color text.
        bool verbose = false; // Display verbose information.

        // TODO: Support Unicode
        bool quite = false; // Search a file until a match has been found.
        bool utf8 = false;  // Support UTF8.
        bool utf16 = false; // Support UTF16.
        bool utf32 = false; // Support UTF32.

        auto cli =
            clara::Help(help) | clara::Opt(verbose)["-v"]["--verbose"]("Display verbose information") |
            clara::Opt(exact_match)["--exact-match"]("Use exact matching algorithms.") |
            clara::Opt(inverse_match)["--invert-match"]("Print lines that do not match given pattern.") |
            clara::Opt(ignore_case)["-i"]["--ignore-case"](
                "Perform case insensitive matching. This is off by default.") |
            clara::Opt(recursive)["-r"]["-R"]["--recursive"]("Recursively search subdirectories listed.") |
            clara::Opt(use_memmap)["--mmap"]("Use mmap to read the file content instead of read. This "
                                             "approach does not work well for big files.") |
            clara::Opt(color)["-c"]["--color"]("Print out color text. This option is off by default.") |
            clara::Opt(linenum)["-n"]["--linenum"]("Display line number. This option is off by default.") |
            clara::Opt(quite)["-q"]["--quite"](
                "Search a file until a match has been found. This option is off by default.") |
            clara::Opt(stdin)["-s"]["--stdin"]("Read data from the STDIN.") |
            clara::Opt(utf8)["--utf8"]("Support UTF8 (WIP).") |
            clara::Opt(utf16)["--utf16"]("Support UTF16 (WIP).") |
            clara::Opt(utf32)["--utf32"]("Support UTF32 (WIP).") |
            clara::Opt(params.pattern, "pattern")["-e"]["-E"]["--pattern"]["--regexp"]("Search pattern.") |
            clara::Opt(params.path_pattern, "path_pattern")["-p"]["--path-regex"]("Path regex.") |

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
            copyright();
            fmt::print("{}", oss.str());
            exit(EXIT_SUCCESS);
        }

        // Update search parameters
        params.parameters.regex_mode =
            HS_FLAG_DOTALL | HS_FLAG_SINGLEMATCH | (ignore_case ? HS_FLAG_CASELESS : 0);
        params.parameters.info = verbose * fastgrep::VERBOSE | color * fastgrep::COLOR |
                                 linenum * fastgrep::LINENUM | utf8 * fastgrep::UTF8 |
                                 use_memmap * fastgrep::USE_MEMMAP | exact_match * fastgrep::EXACT_MATCH |
                                 inverse_match * fastgrep::INVERSE_MATCH | stdin * fastgrep::STDIN |
                                 recursive * fastgrep::RECURSIVE;

        // If users do not specify the search pattern then the first elements of paths is the search
        // pattern.
        if (params.pattern.empty()) {
            if (!stdin && params.paths.size() < 2) {
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

// TODO: Find all files in the given paths.
template <typename T> void fgrep(const InputParams &params) {
    // Find all files that need to search.
    ioutils::search::Params find_params;
    find_params.flags |= ioutils::search::IGNORE_SYMLINK | ioutils::search::IGNORE_DIR;
    find_params.regex = params.path_pattern;

    if (find_params.regex.empty()) {
        using Policy = ioutils::StorePolicy;
        using Search = typename ioutils::FileSearch<Policy>;
        Search search(find_params);
        search.traverse(params.paths);
        T grep(params.pattern, params.parameters);
        auto const &all_paths = search.get_paths();
        for (auto const &afile : all_paths) {
            grep(afile.data());
        }
    } else {
        using Matcher = utils::hyperscan::RegexMatcher;
        using Policy = ioutils::RegexStorePolicy<Matcher>;
        using Search = typename ioutils::FileSearch<Policy>;
        Search search(find_params);
        search.traverse(params.paths);
        T grep(params.pattern, params.parameters);
        auto const &all_paths = search.get_paths();
        for (auto const &afile : all_paths) {
            grep(afile.data());
        }
    }
    // Grep all found files.
}

// grep for desired lines from STDIN
template <typename T> void fgrep_stdin(const InputParams &params) {
    T grep(params.pattern, params.parameters);
    grep(STDIN_FILENO);
}

int main(int argc, char *argv[]) {
    auto params = parse_input_arguments(argc, argv);
    constexpr int BUFFER_SIZE = 1 << 16;

    // Search for given pattern based on input parameters
    if (params.parameters.exact_match()) {
        if (!params.parameters.inverse_match()) {
            using Matcher = utils::ExactMatcher;
            if (params.parameters.use_memmap()) {
                using Policy = typename fastgrep::SimplePolicy<Matcher>;
                using Reader = ioutils::MMapReader<Policy>;
                fgrep<Reader>(params);
            } else {
                using Policy = fastgrep::StreamPolicy<Matcher>;
                params.parameters.stdin() ? fgrep_stdin<ioutils::StreamReader<Policy, BUFFER_SIZE>>(params)
                                          : fgrep<ioutils::FileReader<Policy, BUFFER_SIZE>>(params);
            }
        } else {
            using Matcher = utils::ExactMatcherInv;
            if (params.parameters.use_memmap()) {
                using Policy = typename fastgrep::SimplePolicy<Matcher>;
                using Reader = ioutils::MMapReader<Policy>;
                fgrep<Reader>(params);
            } else {
                using Policy = fastgrep::StreamPolicy<Matcher>;
                params.parameters.stdin() ? fgrep_stdin<ioutils::StreamReader<Policy, BUFFER_SIZE>>(params)
                                          : fgrep<ioutils::FileReader<Policy, BUFFER_SIZE>>(params);
            }
        }
    } else {
        if (!params.parameters.inverse_match()) {
            using Matcher = utils::hyperscan::RegexMatcher;
            if (params.parameters.use_memmap()) {
                using Policy = typename fastgrep::SimplePolicy<Matcher>;
                using Reader = ioutils::MMapReader<Policy>;
                fgrep<Reader>(params);
            } else {
                using Policy = typename fastgrep::StreamPolicy<Matcher>;
                params.parameters.stdin() ? fgrep_stdin<ioutils::StreamReader<Policy, BUFFER_SIZE>>(params)
                                          : fgrep<ioutils::FileReader<Policy, BUFFER_SIZE>>(params);
            }
        } else {
            using Matcher = utils::hyperscan::RegexMatcherInv;
            if (params.parameters.use_memmap()) {
                using Policy = typename fastgrep::SimplePolicy<Matcher>;
                using Reader = ioutils::MMapReader<Policy>;
                fgrep<Reader>(params);
            } else {
                using Policy = typename fastgrep::StreamPolicy<Matcher>;
                params.parameters.stdin() ? fgrep_stdin<ioutils::StreamReader<Policy, BUFFER_SIZE>>(params)
                                          : fgrep<ioutils::FileReader<Policy, BUFFER_SIZE>>(params);
            }
        }
    }

    return EXIT_SUCCESS;
}
