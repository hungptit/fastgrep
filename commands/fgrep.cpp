#include "boost/program_options.hpp"
#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include "utils/regex_matchers.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"

int main(int argc, char *argv[]) {
    if (argc < 3) return EXIT_FAILURE;

    // Matchers
    using Matcher1 = utils::avx2::ContainsStd;
    using Matcher2 = utils::avx2::Contains;
    using Matcher3 = utils::hyperscan::RegexMatcher;

    using Reader = ioutils::FileReader<fastgrep::GrepPolicy<Matcher2>, 1 << 17>;
    Reader grep(argv[1]);
    for (auto idx = 2; idx < argc; ++idx) { grep(argv[idx]); }
    return EXIT_SUCCESS;
}
