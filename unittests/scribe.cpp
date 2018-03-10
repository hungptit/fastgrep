#include "fmt/format.h"
#include <string>

#include "ioutils/ioutils.hpp"

#include "constraints.hpp"
#include "parser.hpp"
#include "scribe.hpp"

#include "header_parser.hpp"

#include <time.h>

#include "utils/timeutils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

TEST_CASE("Parse scribe timstamp", "") {
    auto val1 = scribe::ScribeTimestampParser::parse_two_digits("12/");
    auto val2 = scribe::ScribeTimestampParser::parse_four_digits("2018 ");
    REQUIRE(val1 == 12);
    REQUIRE(val2 == 2018);

	char buf[255];
    const std::string timestamp("03/08/2018 12:00:00");
    scribe::ScribeTimestampParser time_parser;
    std::time_t t = time_parser(&timestamp[0]);
    utils::TimePrinter printer("%Y-%m-%d %H:%M:%S");
    printer(t);
	REQUIRE(strcmp(printer.buffer, "2018-03-08 12:00:00") == 0);

    fmt::print("{0} <---> {1}\n", timestamp, t);
    fmt::print("{0} <---> {1}\n", timestamp, printer.buffer);
}
