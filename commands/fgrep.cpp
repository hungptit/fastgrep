#include "boost/program_options.hpp"
#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include <algorithm>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc < 3) return EXIT_FAILURE;
    using Reader = ioutils::FileReader<fastgrep::GrepPolicyNew, 1 << 20>;
    Reader grep(argv[1]);
    for (auto idx = 2; idx < argc; ++idx) { grep(argv[idx]); }
    return EXIT_SUCCESS;
}
