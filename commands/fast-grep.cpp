#include "fast-grep-params.hpp"
#include "fast-grep.hpp"
#include "fmt/format.h"
#include "ioutils/reader.hpp"
#include "ioutils/search.hpp"
#include "ioutils/stream.hpp"
#include "stream.hpp"
#include "utils/matchers.hpp"
#include "utils/regex_matchers.hpp"
#include <string>
#include <vector>

namespace {
    constexpr int BUFFER_SIZE = 1 << 16;
    template <typename Params> void search(Params &&params) {
        if (!params.exact_match()) {
            if (!params.invert_match()) {
                using GrepMatcher = utils::hyperscan::RegexMatcher;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using GrepReaderPolicy = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;
                
                if (!params.use_mmap()) {
                    using Reader = ioutils::FileReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);
                } else {
                    using Reader = ioutils::MMapReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);
                }
            } else {
                using GrepMatcher = utils::hyperscan::RegexMatcherInv;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using GrepReaderPolicy = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;

                if (!params.use_mmap()) {
                    using Reader = ioutils::FileReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);
                } else {
                    using Reader = ioutils::MMapReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);
                }
            }
        } else {
            if (!params.invert_match()) {
                using GrepMatcher = utils::ExactMatcher;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using GrepReaderPolicy = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;
                if (!params.use_mmap()) {
                    using Reader = ioutils::FileReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);
                } else {
                    using Reader = ioutils::MMapReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);
                }
            } else {
                using GrepMatcher = utils::ExactMatcherInv;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using GrepReaderPolicy = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;
                if (!params.use_mmap()) {
                    using Reader = ioutils::FileReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);
                } else {
                    using Reader = ioutils::MMapReader<GrepReaderPolicy>;
                    using SearchPolicy = fastgrep::experiments::GrepSearchPolicy<Reader>;
                    using Search = ioutils::filesystem::DefaultSearch<SearchPolicy>;
                    Search grep(params);
                    grep.traverse(params.paths);                    
                }
            }
        }
    }

    template <typename Params> void search_stdin(Params &&params) {
        if (!params.exact_match()) {
            if (!params.invert_match()) {
                using GrepMatcher = utils::hyperscan::RegexMatcher;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using Search = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;
                Search grep(params.regex, params);
                grep(STDIN_FILENO);
            } else {
                using GrepMatcher = utils::hyperscan::RegexMatcherInv;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using Search = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;
                Search grep(params.regex, params);
                grep(STDIN_FILENO);
            }
        } else {
            if (!params.invert_match()) {
                using GrepMatcher = utils::ExactMatcher;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using Search = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;
                Search grep(params.regex, params);
                grep(STDIN_FILENO);
            } else {
                using GrepMatcher = utils::ExactMatcherInv;
                using GrepPolicy = fastgrep::StreamPolicy<GrepMatcher>;
                using Search = ioutils::StreamReader<GrepPolicy, BUFFER_SIZE>;
                Search grep(params.regex, params);
                grep(STDIN_FILENO);
            }
        }
    }
} // namespace

int main(int argc, char *argv[]) {
    auto params = fastgrep::experiments::parse_input_arguments(argc, argv);
    if (!params.stdin()) {
        search(params);
    } else { // Grep data from STDIN
        search_stdin(params);
    }
    return EXIT_SUCCESS;
}
