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

    // Matchers
    using Matcher1 = utils::experiments::ExactMatch;
    using Matcher2 = utils::experiments::ExactMatchSSE2;
    // using Matcher3 = utils::avx2::Contains;
    using Matcher4 = utils::hyperscan::RegexMatcher;
    using Matcher5 = utils::experiments::RegexMatcher;

    // using Reader = ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher1>, 1 << 17>;
    // using Reader = ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher2>, 1 << 17>;
    // using Reader = ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher3>, 1 << 17>;
    // using Reader = ioutils::FileReader<fastgrep::experiments::GrepPolicy<Matcher4>, 1 << 17>;
    // using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher4>, 1 << 17>;
    using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher5>, 1 << 17>;
    Reader grep(argv[1]);
    for (auto idx = 2; idx < argc; ++idx) { grep(argv[idx]); }
    return EXIT_SUCCESS;
}
