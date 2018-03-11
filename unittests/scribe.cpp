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

TEST_CASE("Parse scribe timestamp", "") {
    auto val1 = scribe::ScribeTimestampParser::parse_two_digits("12/");
    auto val2 = scribe::ScribeTimestampParser::parse_four_digits("2018 ");
    CHECK(val1 == 12);
    CHECK(val2 == 2018);

    char buf[255];
    const std::string timestamp("03/08/2018 12:00:00");
    scribe::ScribeTimestampParser time_parser;
    std::time_t t = time_parser(&timestamp[0]);
    utils::TimePrinter printer("%Y-%m-%d %H:%M:%S");
    printer(t);
    CHECK(strcmp(printer.buffer, "2018-03-08 12:00:00") == 0);

    fmt::print("{0} <---> {1}\n", timestamp, t);
    fmt::print("{0} <---> {1}\n", timestamp, printer.buffer);
}

TEST_CASE("Parser a header", "") {
    const std::string header("[03/08/2018 12:00:00 node1234.example.com generic.workqueue 123456] {}");
    fmt::print("Header: {}\n", header);
    const char *begin = &header[0];
    const char *end = begin + header.size();
    scribe::MessageHeaderParser parser;
    scribe::MessageHeader msg = parser(begin, end);
    utils::TimePrinter printer("%Y-%m-%d %H:%M:%S");
    printer(msg.timestamp);
    fmt::print("{0} <---> [{1} {2} {3} {4}]\n", header, printer.buffer, msg.server, msg.pool, msg.pid);
    CHECK(strcmp(printer.buffer, "2018-03-08 12:00:00") == 0);
    CHECK(strcmp(msg.server, "node1234.example.com") == 0);
    CHECK(strcmp(msg.pool, "generic.workqueue") == 0);
    CHECK(msg.pid == 123456);
}
