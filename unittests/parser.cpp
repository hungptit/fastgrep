#include "fmt/format.h"
#include <string>

#include "ioutils/ioutils.hpp"
#include "grep.hpp"
#include "params.hpp"
#include "stream.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/regex_matchers.hpp"
#include <time.h>

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

std::string datafile("data.log");

TEST_CASE("Exact match using SSE2") {
    using Matcher = utils::sse2::ExactMatcher;
    using Policy = fastgrep::StreamPolicy<Matcher>;
    fastgrep::Params params{};
    ioutils::MMapReader<Policy> reader("This", params);
    reader(datafile.data());
}

TEST_CASE("Exact matcher") {
    using Matcher = utils::ExactMatcher;
    using Policy = fastgrep::StreamPolicy<Matcher>;
    fastgrep::Params params{};
    ioutils::MMapReader<Policy> reader("This", params);
    reader(datafile.data());
}

TEST_CASE("regex hyperscan", "") {
    using Matcher = utils::hyperscan::RegexMatcher;
    using Policy = fastgrep::StreamPolicy<Matcher>;
    fastgrep::Params params{};
    ioutils::MMapReader<Policy> reader("This.*first", params);
    reader(datafile.data());
}
