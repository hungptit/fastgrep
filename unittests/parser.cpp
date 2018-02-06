#include "fmt/format.h"
#include <string>

#include "ioutils.hpp"
#include "scribe.hpp"
#include <time.h>

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

const std::string datafile("data.log");

TEST_CASE("Basic tests", "") {
    std::string buffer = ioutils::read<std::string>(datafile.c_str());
    using Parser = scribe::BasicParser;
    auto parser = Parser();
	auto begin = buffer.begin();
	auto end = buffer.end();
	fmt::print("len = {}\n", std::distance(begin, end));
    parser(buffer.begin(), buffer.end());

	{
		std::string buff("02/04/2018 23:42:22");
		utils::TimeParser time_parser("%m/%d/%Y %H:%M:%S");
		std::time_t timestamp = time_parser(buff.c_str());
		utils::TimePrinter printer("%Y-%m-%d %H:%M:%S");
		printer(timestamp);
		fmt::print("{0} - {1}\n", buff, printer.buffer);
		fmt::print("{0} - {1}\n", timestamp, buff);
	}
}
