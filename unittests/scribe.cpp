#include "fmt/format.h"
#include <string>

#include "ioutils/ioutils.hpp"
#include "parser.hpp"
#include "scribe.hpp"

#include "utils/timeutils.hpp"
#include "utils/dumper.hpp"

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

using Catch::Matchers::Equals;

// TEST_CASE("Parser a header", "") {
//     const std::string header("[03/08/2018 12:00:00 node1234.example.com generic.workqueue 123456] {}");
//     fmt::print("Header: {}\n", header);
//     const char *begin = &header[0];
//     const char *end = begin + header.size();
//     scribe::MessageHeaderParser parser;
//     scribe::MessageHeader msg = parser(begin, end);
//     utils::TimePrinter printer("%Y-%m-%d %H:%M:%S");
//     printer(msg.timestamp);
//     fmt::print("{0} <---> [{1} {2} {3} {4}]\n", header, printer.buffer, msg.server, msg.pool, msg.pid);
//     CHECK(strcmp(printer.buffer, "2018-03-08 12:00:00") == 0);
//     CHECK_THAT(msg.server, Equals("node1234.example.com"));
//     CHECK_THAT(msg.pool, Equals("generic.workqueue"));
//     CHECK(msg.pid == 123456);
// }

TEST_CASE("Parser scribe body", "") {
	const std::string datafile("body.received.json");
	std::string buffer = ioutils::read<std::string>(datafile.c_str());
	fmt::print("{}\n", buffer);
	
	scribe::MessageBodyParser parser;
	char *begin = const_cast<char*>(buffer.c_str());
	parser(begin, buffer.size());
}

// TEST_CASE("Parser a scribe message", "") {
//     const std::string header1("[03/08/2018 12:00:00 node1234.example.com generic.work 123456] {}");
//     const std::string header2("[03/08/2018 13:00:00 node123456.example.com generic.work.cron 123456] {}");
//     const char *begin = &header1[0];
//     const char *end = begin + header1.size();
//     scribe::MessageHeaderParser parser;
//     scribe::MessageHeader msg1 = parser(begin, end);
//     scribe::MessageHeader msg2 = parser(&header2[0], &header2[0] + header2.size());

//     utils::data_dumper<cereal::JSONOutputArchive>(msg1, "Header1");
//     utils::data_dumper<cereal::JSONOutputArchive>(msg2, "Header2");

//     CHECK_THAT(msg1.server, Equals("node1234.example.com"));
//     CHECK_THAT(msg1.pool, Equals("generic.work"));
//     CHECK(msg1.pid == 123456);
//     CHECK(msg1.timestamp == 1520528400);

// }

