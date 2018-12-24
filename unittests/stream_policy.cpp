#include "fmt/format.h"
#include <string>

#include "constants.hpp"
#include "ioutils/ioutils.hpp"
#include "output.hpp"
#include "params.hpp"
#include "stream.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/regex_matchers.hpp"
#include <time.h>

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

std::string single_line_without_eol() {
    std::string data;
    data.append("This is a simple line.");
    return data;
}

std::string multiple_lines_without_eol() {
    std::string data;
    data.append("This is a simple line.");
    data.push_back(fastgrep::EOL);
    data.append("This is another simple line.");
    data.push_back(fastgrep::EOL);
    data.append("This is the last line.");
    return data;
}

std::string multiple_lines_end_without_eol() {
    std::string data;
    data.append("This is a simple line.");
    return data;
}

TEST_CASE("We should be able to construct a stream object") {
    using Matcher = utils::hyperscan::RegexMatcher;
    using Policy = typename fastgrep::StreamPolicy<Matcher>;
    fastgrep::Params params;
    Policy pol("is\\s+a", params);
    auto data = single_line_without_eol();

    // We should be able to see the matched line at the output.
    pol.process(data.data(), data.size());
}

TEST_CASE("Check that we can grep a line without EOL.") {
    using Matcher = utils::hyperscan::RegexMatcher;
    using Policy = typename fastgrep::StreamPolicy<Matcher, fastgrep::StorePolicy>;
    fastgrep::Params params;

    SECTION("Won't be able to have any thing for one line without EOL even if it matched. Input data "
            "should be in the line buffer",
            "Matched") {
        Policy pol("is\\s+a", params);
        auto data = single_line_without_eol();

        // We should be able to see the matched line at the output.
        pol.process(data.data(), data.size());
        fmt::print("Number of lines: {}\n", pol.linebuf);
        REQUIRE(pol.console.lines.size() == 0);
        REQUIRE(data == pol.linebuf);
    }

    SECTION("Won't be able to have any thing for one line without EOL if it does not match. Input data "
            "should be in the line buffer",
            "Matched") {
        Policy pol("hello", params);
        auto data = single_line_without_eol();

        // We should be able to see the matched line at the output.
        pol.process(data.data(), data.size());
        fmt::print("Number of lines: {}\n", pol.console.lines.size());
        REQUIRE(pol.console.lines.size() == 0);
        REQUIRE(data == pol.linebuf);
    }
}

TEST_CASE("Check that we can grep a line with EOL.") {
    using Matcher = utils::hyperscan::RegexMatcher;
    using Policy = typename fastgrep::StreamPolicy<Matcher, fastgrep::StorePolicy>;
    fastgrep::Params params;
    auto data = single_line_without_eol();

    // Append EOL character to the end.
    data.push_back(fastgrep::EOL);

    SECTION("is matched") {
        Policy pol("is\\s+a", params);
        pol.process(data.data(), data.size());
        REQUIRE(pol.console.lines.size() == 1);
        REQUIRE(pol.linebuf.empty());
    }

    SECTION("is not matched") {
        Policy pol("hello", params);
        pol.process(data.data(), data.size());
        REQUIRE(pol.console.lines.size() == 0);
        REQUIRE(pol.linebuf.empty());
    }
}

TEST_CASE("Multiple lines without EOL at the end.") {
    using Matcher = utils::hyperscan::RegexMatcher;
    using Policy = typename fastgrep::StreamPolicy<Matcher, fastgrep::StorePolicy>;
    fastgrep::Params params;
    auto data = multiple_lines_without_eol();

    SECTION("is matched") {
        Policy pol("is\\s+(a|t)", params);
        pol.process(data.data(), data.size());
        REQUIRE(pol.console.lines.size() == 2);
        REQUIRE(!pol.linebuf.empty());
    }

    SECTION("is not matched") {
        Policy pol("hello", params);
        pol.process(data.data(), data.size());
        REQUIRE(pol.console.lines.size() == 0);
        REQUIRE(!pol.linebuf.empty());
    }
}

TEST_CASE("Multiple lines with EOL at the end.") {
    using Matcher = utils::hyperscan::RegexMatcher;
    using Policy = typename fastgrep::StreamPolicy<Matcher, fastgrep::StorePolicy>;
    fastgrep::Params params;
    auto data = multiple_lines_without_eol();

    // Append EOL character to the end.
    data.push_back(fastgrep::EOL);

    SECTION("is matched") {
        Policy pol("is\\s+(a|t)", params);
        pol.process(data.data(), data.size());
        REQUIRE(pol.console.lines.size() == 3);
        REQUIRE(pol.linebuf.empty());
    }

    SECTION("is not matched") {
        Policy pol("hello", params);
        pol.process(data.data(), data.size());
        REQUIRE(pol.console.lines.size() == 0);
        REQUIRE(pol.linebuf.empty());
    }
}
