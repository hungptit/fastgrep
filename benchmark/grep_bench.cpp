#include "celero/Celero.h"

#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include <algorithm>
#include <iostream>
#include <string>

constexpr int number_of_samples = 10;
constexpr int number_of_operations = 1;

CELERO_MAIN

const std::string datafile("3200.txt");
const std::string pattern1 = "Twain";
const std::string pattern2 = "(?i)Twain";
const std::string pattern3 = "[a-z]shing";
const std::string pattern4 = "Huck[a-zA-Z]+|Saw[a-zA-Z]+";
const std::string pattern5 = "\b\\w+nn\b";
const std::string pattern6 = "[a-q][^u-z]{13}x";
const std::string pattern7 = "Tom|Sawyer|Huckleberry|Finn";

namespace {
    void exec(const std::string &command, const std::string &pattern) {
        std::string cmd = command + " '" + pattern + "' " + datafile + " > output.log";
        system(cmd.c_str());
    }
} // namespace

const std::string fast_grep = "../commands/fast-grep ";
#ifdef __APPLE__
const std::string gnu_grep = "ggrep -E ";
#else
const std::string gnu_grep = "grep -E ";
#endif
const std::string rip_grep = "rg ";
const std::string ag = "ag";

BASELINE(pattern1, gnu_grep, number_of_samples, number_of_operations) {
    exec("grep", pattern1);
}

BENCHMARK(pattern1, ag, number_of_samples, number_of_operations) { exec("ag", pattern1); }

BENCHMARK(pattern1, ripgrep, number_of_samples, number_of_operations) { exec(rip_grep, pattern1); }

BENCHMARK(pattern1, fgrep, number_of_samples, number_of_operations) {
    exec(fast_grep, pattern1);
}

// BENCHMARK(pattern1, fastgrep_exact_match, number_of_samples, number_of_operations) {
//     exec("../commands/fastgrep --no-regex", pattern1);
// }

// BENCHMARK(pattern1, fastgrep, number_of_samples, number_of_operations) {
//     exec("../commands/fastgrep", pattern1);
// }

// pattern2
BASELINE(pattern2, gnu_grep, number_of_samples, number_of_operations) {
    exec(gnu_grep, pattern2);
}

BENCHMARK(pattern2, ag, number_of_samples, number_of_operations) { exec("ag", pattern2); }

BENCHMARK(pattern2, ripgrep, number_of_samples, number_of_operations) { exec(rip_grep, pattern2); }

BENCHMARK(pattern2, fgrep, number_of_samples, number_of_operations) {
    exec(fast_grep, pattern2);
}

// BENCHMARK(pattern2, fastgrep, number_of_samples, number_of_operations) {
//     exec("../commands/fastgrep", pattern2);
// }

// pattern3
BASELINE(pattern3, gnu_grep, number_of_samples, number_of_operations) {
    exec(gnu_grep, pattern3);
}

BENCHMARK(pattern3, ag, number_of_samples, number_of_operations) { exec("ag", pattern3); }

BENCHMARK(pattern3, ripgrep, number_of_samples, number_of_operations) { exec(rip_grep, pattern3); }

BENCHMARK(pattern3, fgrep, number_of_samples, number_of_operations) {
    exec(fast_grep, pattern3);
}

// BENCHMARK(pattern3, fastgrep, number_of_samples, number_of_operations) {
//     exec(fast_grep, pattern3);
// }

// pattern4
BASELINE(pattern4, grep, number_of_samples, number_of_operations) {
    exec(gnu_grep, pattern4);
}

BENCHMARK(pattern4, ag, number_of_samples, number_of_operations) { exec("ag", pattern4); }

BENCHMARK(pattern4, ripgrep, number_of_samples, number_of_operations) { exec(rip_grep, pattern4); }

BENCHMARK(pattern4, fgrep, number_of_samples, number_of_operations) {
    exec(fast_grep, pattern4);
}

// BENCHMARK(pattern4, fastgrep, number_of_samples, number_of_operations) {
//     exec("../commands/fastgrep", pattern4);
// }

// pattern5
BASELINE(pattern5, grep, number_of_samples, number_of_operations) {
    exec(gnu_grep, pattern5);
}

BENCHMARK(pattern5, ag, number_of_samples, number_of_operations) { exec("ag", pattern5); }

BENCHMARK(pattern5, ripgrep, number_of_samples, number_of_operations) { exec(rip_grep, pattern5); }

BENCHMARK(pattern5, fgrep, number_of_samples, number_of_operations) {
    exec(fast_grep, pattern5);
}

// BENCHMARK(pattern5, fastgrep, number_of_samples, number_of_operations) {
//     exec("../commands/fastgrep", pattern5);
// }

// pattern6
BASELINE(pattern6, grep, number_of_samples, number_of_operations) {
    exec(gnu_grep, pattern6);
}

BENCHMARK(pattern6, ag, number_of_samples, number_of_operations) { exec("ag", pattern6); }

BENCHMARK(pattern6, ripgrep, number_of_samples, number_of_operations) { exec(rip_grep, pattern6); }

BENCHMARK(pattern6, fgrep, number_of_samples, number_of_operations) {
    exec(fast_grep, pattern6);
}

// BENCHMARK(pattern6, fastgrep, number_of_samples, number_of_operations) {
//     exec("../commands/fastgrep", pattern6);
// }

// pattern7
BASELINE(pattern7, grep, number_of_samples, number_of_operations) {
    exec(gnu_grep, pattern7);
}

BENCHMARK(pattern7, ag, number_of_samples, number_of_operations) { exec("ag", pattern7); }

BENCHMARK(pattern7, ripgrep, number_of_samples, number_of_operations) { exec(rip_grep, pattern7); }

BENCHMARK(pattern7, fgrep, number_of_samples, number_of_operations) {
    exec(fast_grep, pattern7);
}

// BENCHMARK(pattern7, fastgrep, number_of_samples, number_of_operations) {
//     exec("../commands/fastgrep", pattern7);
// }

