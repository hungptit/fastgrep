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
const std::string pattern2 = "Foooooo";
const std::string pattern3 = "Foooooo";
const std::string pattern4 = "Foooooo";

BASELINE(grep_pattern1, gnu_grep, number_of_samples, number_of_operations) {
    std::string cmd = std::string("grep ") + pattern1 + " " + datafile + " > /dev/null";
    system(cmd.c_str());
}

BENCHMARK(grep_pattern1, ripgrep, number_of_samples, number_of_operations) {
    std::string cmd = std::string("rg ") + pattern1 + " " + datafile + " > /dev/null";
    system(cmd.c_str());
}

BENCHMARK(grep_pattern1, fgrep, number_of_samples, number_of_operations) {
    std::string cmd = std::string("../commands/fgrep ") + pattern1 + " " + datafile + " > /dev/null";
    system(cmd.c_str());
}

BENCHMARK(grep_pattern1, fastgrep, number_of_samples, number_of_operations) {
    std::string cmd = std::string("../commands/fastgrep --no-regex ") + pattern1 + " " + datafile + " > /dev/null";
    system(cmd.c_str());
}
