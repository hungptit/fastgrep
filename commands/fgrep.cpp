#include "boost/program_options.hpp"
#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include "utils/regex_matchers.hpp"
#include "utils/matchers.hpp"
// #include "utils/matchers_avx2.hpp"

int main(int argc, char *argv[]) {
    if (argc < 3) return EXIT_FAILURE;
    constexpr int BUFFER_SIZE = 1 << 16;

    // Matchers
    using Matcher1_1 = utils::experiments::ExactMatch;
    using Matcher2_1 = utils::experiments::ExactMatchSSE2;
    // using Matcher3_1 = utils::avx2::Contains;


    using Matcher1_2 = utils::ExactMatchSSE2;
    // using Matcher2_2 = utils::avx2::Contains;
    using Matcher3_2 = utils::experiments::RegexMatcher;
    using Matcher4_2 = utils::hyperscan::RegexMatcher;

    // Simple parser
    using Reader11 = ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher1_1>, BUFFER_SIZE>;
    using Reader12 = ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher2_1>, BUFFER_SIZE>;
    // using Reader13 = ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher3_1>, BUFFER_SIZE>;

    // Improve parser
    using Reader21 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher1_2>, BUFFER_SIZE>;
    // using Reader22 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher2_2>, BUFFER_SIZE>;
    using Reader23 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher3_2>, BUFFER_SIZE>;
    using Reader24 = ioutils::FileReader<fastgrep::GrepPolicy<Matcher4_2>, BUFFER_SIZE>;

    // Grep the content of given files.
    Reader24 grep(argv[1]);
    for (auto idx = 2; idx < argc; ++idx) { grep(argv[idx]); }
    return EXIT_SUCCESS;
}
