#include "celero/Celero.h"

#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include <algorithm>
#include <iostream>
#include <string>

constexpr int number_of_samples = 5;
constexpr int number_of_operations = 1;

CELERO_MAIN

const std::string datafile("3200.txt");
const std::vector<std::string> patterns = {
    "Twain",
    "(?i)Twain",
    "[a-z]shing",
    "Huck[a-zA-Z]+|Saw[a-zA-Z]+",
    "\b\\w+nn\b",
    "[a-q][^u-z]{13}x",
    "Tom|Sawyer|Huckleberry|Finn",
    "(?i)Tom|Sawyer|Huckleberry|Finn",
    ".{0,2}(Tom|Sawyer|Huckleberry|Finn)",
    ".{2,4}(Tom|Sawyer|Huckleberry|Finn)",
    "Tom.{10,25}river|river.{10,25}Tom",
    "[a-zA-Z]+ing",
    "\\s[a-zA-Z]{0,12}ing\\s",
    "([A-Za-z]awyer|[A-Za-z]inn)\\s",
    // "[\\"'][^\\"']{0,30}[?!\\.][\"']",
    "\\p{Sm}",
};

namespace {
    void run_a_test(const std::string datafile, const std::string &command,
                    const std::string &pattern) {
        std::string cmd = command + " '" + pattern + "' " + datafile + " > output.log";
        system(cmd.c_str());
    }
    void run_all_tests(const std::string datafile, const std::string &command,
                       const std::vector<std::string> &patterns) {
        for (auto const &item : patterns) { run_a_test(datafile, command, item); }
    }
} // namespace

BASELINE(mark_twain, grep, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "grep -E", patterns);
}


BENCHMARK(mark_twain, ag, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "ag", patterns);
}

BENCHMARK(mark_twain, ripgrep, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "rg", patterns);
}

BENCHMARK(mark_twain, fgrep_mmap, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "../commands/fgrep --mmap ", patterns);
}

BENCHMARK(mark_twain, fgrep_stream, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "../commands/fgrep --stream ", patterns);
}

BENCHMARK(mark_twain, fgrep_default, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "../commands/fgrep ", patterns);
}
