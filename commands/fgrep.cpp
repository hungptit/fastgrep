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

// cereal
#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

/**
 * The grep execution process has two steps:
 * 1. Expand the search paths and get the list of search files.
 * 2. Search for given pattern using file contents and display the search results.
 */
namespace {
    struct InputParams {
        std::string pattern;            // Grep pattern
        std::string path_pattern;       // Search path pattern
        std::vector<std::string> paths; // Input files and folders
        fastgrep::Params parameters;    // Grep parameters

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(pattern), CEREAL_NVP(paths), CEREAL_NVP(parameters));
        }
    };

    // Use clara to parse input argument.
    InputParams parse_input_arguments(int argc, char *argv[]) {
        InputParams params;
        bool help;
        auto cli =
            clara::Help(help) |
            clara::Opt(params.parameters.verbose)["-v"]["--verbose"]("Display verbose information") |
            clara::Opt(params.parameters.inverse_match)["--inverse-match"](
                "Only print out lines that do not match the search pattern.") |
            clara::Opt(params.parameters.exact_match)["--exact-match"]("Use exact matching algorithms.") |
            clara::Opt(params.parameters.ignore_case)["-i"]["--ignore-case"]("Ignore case") |

            clara::Opt(params.parameters.stream)["--pipe"]("Get data from the input pipe/stream.") |
            clara::Opt(params.parameters.color)["-c"]["--color"]("Print out color text.") |
            clara::Opt(params.parameters.utf8)["--utf8"]("Support UTF8.") |

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

        // Display input arguments in JSON format if verbose flag is on
        if (true) {
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

template <typename T> void fgrep(const InputParams &params) {
    T grep(params.pattern.data());
    for (auto afile : params.paths) { grep(afile.data()); }
}

int main(int argc, char *argv[]) {
    constexpr int BUFFER_SIZE = 1 << 17;
    auto params = parse_input_arguments(argc, argv);

    // Search for given pattern based on input parameters
    // if (params.parameters.exact_match) {
    //     // using Matcher = utils::ExactMatchSSE2;
    //     using Matcher = utils::ExactMatchAVX2;
    //     if (params.parameters.use_memmap) {
    //         using Reader = ioutils::MMapReader<fastgrep::GrepPolicy<Matcher>>;
    //         fgrep<Reader>(params);
    //     } else {
    //         using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher>, BUFFER_SIZE>;
    //         fgrep<Reader>(params);
    //     }
    // } else {
    //     using Matcher = utils::hyperscan::RegexMatcher;
    //     if (params.parameters.use_memmap) {
    //         using Reader = ioutils::MMapReader<fastgrep::GrepPolicy<Matcher>>;
    //         fgrep<Reader>(params);
    //     } else {
    //         using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher>, BUFFER_SIZE>;
    //         fgrep<Reader>(params);
    //     }
    // }

    return EXIT_SUCCESS;
}
