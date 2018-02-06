#include "fmt/format.h"
#include <string>

#include "ioutils.hpp"
#include "rabbitmq.hpp"
#include <time.h>

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

const std::string datafile("data.log");

TEST_CASE("Basic tests", "") {
    std::string buffer = ioutils::read<std::string>(datafile.c_str());
    using Parser = rabbitmq::Parser;
    using Container = typename Parser::Container;
    using Iter = Container::const_iterator;
    auto parser = Parser();
    parser(buffer.begin(), buffer.end(), rabbitmq::AllPass());
    REQUIRE(parser.messages.size() == 6);

    // Print out parsed data
    rabbitmq::print_messages<rabbitmq::All, Iter>(parser.messages.cbegin(), parser.messages.cend());

    // Check the number of error and info messages.
    size_t error(0), info(0);
    for (auto const msg : parser.messages) {
        info += msg.error_code == rabbitmq::ErrorCode::INFO;
        error += msg.error_code == rabbitmq::ErrorCode::ERROR;
    }

    REQUIRE(info == 5);
    REQUIRE(error == 1);
    REQUIRE(parser.messages[0].error_code == rabbitmq::ErrorCode::INFO);
    // REQUIRE(parser.messages[0].timestamp == 1516891170);
    REQUIRE(parser.messages[0].message == "Adding mirror of queue "
                                          "'aggregator.forward.WmnsIKwcGBEAAHOkAAAA5A' in vhost 'DB1' on "
                                          "node rabbit@mq103: <22210.12697.3895>");
}

TEST_CASE("Parsing time strings", "Positive") {
    rabbitmq::TimeParser parser("%Y-%m-%d %H:%M:%S");
    std::time_t t = parser("2018-1-11 10:30:05");
    rabbitmq::TimePrinter printer("%Y-%m-%d %H:%M:%S");
    printer(t);
    fmt::print("parsed time: {}\n", printer.buffer);
    std::string expected_results("2018-01-11 10:30:05");
    REQUIRE(expected_results == printer.buffer); // TODO: Need to normalize time.
}

TEST_CASE("Comparators", "Positive") {
    rabbitmq::TimeParser parser("%Y-%m-%d %H:%M:%S");
    std::time_t t1 = parser("2018-10-1 9:10:11");
    std::time_t t2 = parser("2018-1-1 19:10:11");
    std::time_t t3 = parser("2017-1-1 1:10:11");

    rabbitmq::TimePrinter printer("%Y-%m-%d %H:%M:%S");

    printer(t1);
    fmt::print("t1 = {0} -> {1}\n", t1, printer.buffer);

    printer(t2);
    fmt::print("t2 = {0} -> {1}\n", t2, printer.buffer);

    printer(t3);
    fmt::print("t3 = {0} -> {1}\n", t3, printer.buffer);

    SECTION("Olderthan") {
        rabbitmq::OlderThan cons1(t2);
        REQUIRE(cons1(t1) == false);
        REQUIRE(cons1(t3) == true);
    }

    SECTION("NewerThan") {
        rabbitmq::NewerThan cons2(t2);
        REQUIRE(cons2(t1) == true);
        REQUIRE(cons2(t3) == false);
    }

	SECTION("TimePeriod") {
		rabbitmq::TimePeriod cons1(t3, t1);
        REQUIRE(cons1(t2) == true);
		
		rabbitmq::TimePeriod cons2(t2, t1);
        REQUIRE(cons2(t3) == false);
	}
}
