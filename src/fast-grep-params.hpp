#pragma once
#include "clara.hpp"
#include "fmt/format.h"
#include "utils/regex_matchers.hpp"
#include "ioutils/utilities.hpp"

namespace fastgrep {
    namespace experiments {
        enum PARAMS : uint32_t {
            VERBOSE = 1,                 // Display verbose flagsrmation.
            COLOR = 1 << 1,              // Display color text.
            EXACT_MATCH = 1 << 2,        // Use exact string matching algorithms to search for text pattern.
            USE_MEMMAP = 1 << 3,         // Use memory mapped to read the file content.
            UTF8 = 1 << 4,               // Support UTF8
            UTF16 = 1 << 5,              // Support UTF16
            UTF32 = 1 << 6,              // Support UTF32
            LINENUM = 1 << 7,            // Display line numbers
            INVERT_MATCH = 1 << 8,       // Display lines that do not match our search pattern
            STDIN = 1 << 9,              // Read data from the STDIN
            RECURSIVE = 1 << 10,         // Search for files in input paths recursively.
            QUITE = 1 << 11,             // Do not produce any output.
            FOLLOW_SYMLINK = 1 << 12,    // Follow symlinks
            DFS = 1 << 13,               // Use the DFS algorithm for traversing directories.
            SKIP_BINARY_FILES = 1 << 14, // Do not search for lines in binary files.
            IGNORE_ERROR = 1 << 15,      // Do not print out any error messages related to file I/O
        };

        struct Params {
            int flags = 0;
            int regex_mode = 0;
            int maxdepth = std::numeric_limits<int>::max();
            std::string regex;
            std::string path_regex;
            std::string prefix;
            std::vector<std::string> paths;

            bool verbose() const { return (flags & VERBOSE) > 0; }
            bool color() const { return (flags & COLOR) > 0; }
            bool use_mmap() const { return (flags & USE_MEMMAP) > 0; }
            bool exact_match() const { return (flags & EXACT_MATCH) > 0; }
            bool invert_match() const { return (flags & INVERT_MATCH) > 0; }
            bool linenum() const { return (flags & LINENUM) > 0; }
            bool stdin() const { return (flags & STDIN) > 0; }
            bool recursive() const { return (flags & RECURSIVE) > 0; }

            bool dfs() const { return (flags & DFS) > 0; }
            bool follow_symlink() const { return (flags & FOLLOW_SYMLINK) > 0; }
            bool donot_ignore_git() const { return false; }
            bool ignore_error() const { return false; }

            // Unused methods
            bool quite() const { return (flags & QUITE) > 0; }
            bool utf8() const { return (flags & UTF8) > 0; }
            bool utf16() const { return (flags & UTF16) > 0; }
            bool utf32() const { return (flags & UTF32) > 0; }

            // TODO: Only need to add support for params in fmt.
            void print() const {
                fmt::print("verbose: {}\n", verbose());
                fmt::print("color: {}\n", color());
                fmt::print("use_mmap: {}\n", use_mmap());
                fmt::print("stdin: {}\n", stdin());
                fmt::print("exact_match: {}\n", exact_match());
                fmt::print("regex_mode: {}\n", regex_mode);
                fmt::print("linenum: {}\n", linenum());
                fmt::print("recursive: {}\n", recursive());

                fmt::print("utf8: {}\n", utf8());
                fmt::print("utf16: {}\n", utf16());
                fmt::print("utf32: {}\n", utf32());

                fmt::print("regex: {}\n", regex);
                fmt::print("path_regex: {}\n", path_regex);
                if (!prefix.empty()) fmt::print("prefix: {}\n", prefix);

                fmt::print("Input paths: [");
                for (auto item : paths) { fmt::print(" '{}'", item); }
                fmt::print(" ]\n");
            }
        };

        void copyright() {
            fmt::print("{}\n", "fgrep version 0.1.0");
            fmt::print("{}\n", "Hung Dang <hungptit@gmail.com>");
        }

        // Use clara to parse input argument.
        Params parse_input_arguments(int argc, char *argv[]) {
            Params params;

            // Input argument
            bool help = false;
            bool linenum = false;      // Display line number.
            bool invert_match = false; // Inverse match i.e display lines that do not match given pattern.
            bool exact_match = false;  // Use exact matching algorithm.
            bool ignore_case = false;  // Ignore case.
            bool recursive = false;    // Recursively search subdirectories.

            bool use_memmap = false; // Read the file content using memory mapped approach.

            bool stdin = false;   // Read data from STDIN.
            bool color = false;   // Display color text.
            bool verbose = false; // Display verbose information.

            bool follow_symlink = false;

            // TODO: Support Unicode
            bool quite = false; // Search a file until a match has been found.
            bool utf8 = false;  // Support UTF8.
            bool utf16 = false; // Support UTF16.
            bool utf32 = false; // Support UTF32.

            auto cli =
                clara::Help(help) | clara::Opt(verbose)["--verbose"]("Display verbose information") |
                clara::Opt(exact_match)["--exact-match"]("Use exact matching algorithms.") |
                clara::Opt(invert_match)["-v"]["--invert-match"]("Print lines that do not match given pattern.") |
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

                clara::Opt(params.regex, "pattern")["-e"]["-E"]["--pattern"]["--regex"]("Search pattern.") |
                clara::Opt(params.path_regex, "path_pattern")["-p"]["--path-regex"]("Path regex.") |

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

            // If users do not specify the search pattern then the first elements of paths is the search
            // pattern.
            if (params.regex.empty()) {
                if (params.paths.empty()) {
                    throw std::runtime_error("Invalid syntax. The search pattern and search paths are required.");
                } else if (params.paths.size() == 1) {
                    stdin = 1;
                    params.regex = params.paths.back();
                    params.paths.pop_back();
                } else {
                    params.regex = params.paths.front();
                    params.paths.erase(params.paths.begin());
                }
            }

            // Remove the trailing slash
            if (!params.paths.empty()) {
                for (auto & item : params.paths) {
                    ioutils::remove_trailing_slash(item);
                }
            }
            
            // Set the path regex to "." if users do not provide it.
            if (params.path_regex.empty()) { params.path_regex = "."; }

            // Update search parameters
            params.regex_mode = HS_FLAG_DOTALL | HS_FLAG_SINGLEMATCH | (ignore_case ? HS_FLAG_CASELESS : 0);
            params.flags = verbose * VERBOSE | color * COLOR | linenum * LINENUM | utf8 * UTF8 |
                           use_memmap * USE_MEMMAP | exact_match * EXACT_MATCH | invert_match * INVERT_MATCH |
                           stdin * STDIN | recursive * RECURSIVE | follow_symlink * FOLLOW_SYMLINK;

            if (verbose) params.print();

            return params;
        }
    } // namespace experiments
} // namespace fastgrep
