#include "celero/Celero.h"

#include "fmt/format.h"
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
    void run_a_test(const std::string datafile, const std::string &command, const std::string &pattern) {
        std::string cmd = command + " '" + pattern + "' " + datafile + " > output.log";
        system(cmd.c_str());
    }
    void run_all_tests(const std::string datafile, const std::string &command,
                       const std::vector<std::string> &patterns) {
        for (auto const &item : patterns) { run_a_test(datafile, command, item); }
    }

    void test_with_boost_source_code(const std::string command, const std::string options,
                                     const std::string &path) {
        const std::string buffer = command + options + path + " > output.log";
        // fmt::print("{}\n", buffer);
        system(buffer.data());
    }
} // namespace

BASELINE(mark_twain, grep, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "ggrep -E", patterns);
}

BENCHMARK(mark_twain, ag, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "ag", patterns);
}

BENCHMARK(mark_twain, ripgrep_no_mmap, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "rg  -un --no-mmap ", patterns);
}

BENCHMARK(mark_twain, ripgrep_mmap, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "rg  -un --mmap ", patterns);
}

BENCHMARK(mark_twain, ucg, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "ucg", patterns);
}

BENCHMARK(mark_twain, fgrep_mmap, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "../commands/fast-grep --mmap -n ", patterns);
}

BENCHMARK(mark_twain, fgrep_default, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "../commands/fast-grep -n ", patterns);
}

BENCHMARK(mark_twain, fast_grep, number_of_samples, number_of_operations) {
    run_all_tests(datafile, "../commands/fast-grep-new -n ", patterns);
}

const std::string boost_src = "../../3p/src/boost/";
const std::string pattern = " 'coroutine.*Ex\\w+cutor' ";

// We are only interrested in C++ files in this benchmark.
BASELINE(boost_source, grep, number_of_samples, number_of_operations) {
    test_with_boost_source_code("ggrep  -En -r --include='*.cpp' --include='*.hpp' ", pattern, boost_src);
}

BENCHMARK(boost_source, ag, number_of_samples, number_of_operations) {
    test_with_boost_source_code("ag --cpp ", pattern, boost_src);
}

BENCHMARK(boost_source, ripgrep_no_mmap, number_of_samples, number_of_operations) {
    test_with_boost_source_code("rg -un --no-mmap -t cpp --color never ", pattern, boost_src);
}

BENCHMARK(boost_source, ripgrep_mmap, number_of_samples, number_of_operations) {
    test_with_boost_source_code("rg -un --mmap -t cpp --color never ", pattern, boost_src);
}

BENCHMARK(boost_source, ucg, number_of_samples, number_of_operations) {
    test_with_boost_source_code("ucg --noenv --cpp ", pattern, boost_src);
}

BENCHMARK(boost_source, fgrep_mmap, number_of_samples, number_of_operations) {
    test_with_boost_source_code("../commands/fast-grep --mmap -c -n -p '[.](cpp|hpp)' ", pattern, boost_src);
}

BENCHMARK(boost_source, fgrep, number_of_samples, number_of_operations) {
    test_with_boost_source_code("../commands/fast-grep -c -n -p '[.](cpp|hpp)' ", pattern, boost_src);
}

BENCHMARK(boost_source, fast_grep, number_of_samples, number_of_operations) {
    test_with_boost_source_code("../commands/fast-grep-new -c -n -p '[.](cpp|hpp)' ", pattern, boost_src);
}

