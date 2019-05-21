#include "celero/Celero.h"
#include "fmt/format.h"
#include "grep.hpp"
#include "ioutils/ioutils.hpp"
#include <algorithm>
#include <iostream>
#include <string>

constexpr int number_of_operations = 1;

CELERO_MAIN

const std::string logfile("grep_data.log");
const std::vector<std::string> log_patterns = {
    "Starting execution",  "p4pqmewebsync.*finished in (\\d)*",
    "\"LEVEL\":\"error\"", "job1070.athenahealth.com",
    "db.db7.urgent",       "Execution error. Attempting to handle"};

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

// Benchmark log search
constexpr int log_search_samples = 3;
BASELINE(logdata, grep_brew, log_search_samples, number_of_operations) {
    run_all_tests(logfile, "ggrep -E", log_patterns);
}

BENCHMARK(logdata, fgrep_mmap, log_search_samples, number_of_operations) {
    run_all_tests(logfile, "../commands/fgrep --use-mmap=true", log_patterns);
}

BENCHMARK(logdata, fgrep_read, log_search_samples, number_of_operations) {
    run_all_tests(logfile, "../commands/fgrep --use-mmap=false", log_patterns);
}

BENCHMARK(logdata, ag, log_search_samples, number_of_operations) {
    run_all_tests(logfile, "ag", log_patterns);
}

BENCHMARK(logdata, ripgrep, log_search_samples, number_of_operations) {
    run_all_tests(logfile, "rg", log_patterns);
}

BENCHMARK(logdata, ripgrep_nommap, log_search_samples, number_of_operations) {
    run_all_tests(logfile, "rg --no-mmap", log_patterns);
}

BENCHMARK(logdata, fgrep_second, log_search_samples, number_of_operations) {
    run_all_tests(logfile, "../commands/fgrep", log_patterns);
}

// BENCHMARK(logdata, fastgrep, log_search_samples, number_of_operations) {
//     run_all_tests(logfile, "../commands/fastgrep", log_patterns);
// }
