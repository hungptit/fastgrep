#include "fmt/format.h"
#include <string>

#include "ioutils/ioutils.hpp"
#include "grep.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/regex_matchers.hpp"
#include <time.h>

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

std::string datafile("data.log");

// TODO: How to mock fmt::print for testing purpose?

TEST_CASE("Exact match SSE2", "SSE2") {
    using Matcher = utils::ExactMatchSSE2;
    using Policy = fastgrep::GrepPolicy<Matcher>;
    ioutils::MMapReader<Policy> reader("This");
    reader(datafile.data());
}

TEST_CASE("Exact match AVX2", "AVX2") {
    using Matcher = utils::ExactMatchAVX2;
    using Policy = fastgrep::GrepPolicy<Matcher>;
    ioutils::MMapReader<Policy> reader("This");
    reader(datafile.data());
}

TEST_CASE("regex hyperscan", "") {
    using Matcher = utils::hyperscan::RegexMatcher;
    using Policy = fastgrep::GrepPolicy<Matcher>;
    ioutils::MMapReader<Policy> reader("This.*first");
    reader(datafile.data());
}
