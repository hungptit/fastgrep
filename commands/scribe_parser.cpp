#include "fmt/format.h"
#include <iostream>
#include <string>

#include "constraints.hpp"
#include "ioutils/ioutils.hpp"
#include "parser.hpp"
#include "scribe.hpp"
#include <time.h>

#include "boost/program_options.hpp"
#include "utils/matchers.hpp"

namespace {}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    std::vector<std::string> logfiles;
    std::string database;
    po::options_description desc("Allowed options");
    std::string begin_time, end_time;
    std::string pool_pattern, server_pattern;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
		("verbose,v", "Display verbose information i.e input arguments")
		("info,i", "Display information messages.")
		("error,e", "Display error messages.")

		("begin-time", po::value<std::string>(&begin_time), "Begin time in 'yyyy-mm-dd hh::mm::ss' format.")
		("end-time", po::value<std::string>(&end_time), "End time in 'yyyy/mm/dd hh::mm::ss' format")

		("server", po::value<std::string>(&server_pattern), "Server name pattern")
		("pool", po::value<std::string>(&pool_pattern), "Pool name pattern")

        ("log-files,f", po::value<std::vector<std::string>>(&logfiles), "RabbitMQ log files.")
        ("database,d", po::value<std::string>(&database), "A database of rabbitmq logs");
    // clang-format on

    // Parse input arguments
    po::positional_options_description p;
    p.add("log-files", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: scribe_parser [options]\n";
        std::cout << desc;
        std::cout << "\nExamples:\n";
        std::cout << "\n  Display all error message:\n";
        std::cout << "    parse_rabbitmq_log data.log --error\n";
        std::cout << "\n  Display all info message:\n";
        std::cout << "    scribe_parser data.log --info\n";
        return EXIT_SUCCESS;
    }

    if (logfiles.empty()) {
        std::cerr << "You must provide a scribe log file!\n";
        return EXIT_FAILURE;
    }

    bool parse_error(vm.count("error"));
    bool parse_info(vm.count("info"));

    // scribe::TimeAll tcons;
    // utils::baseline::Contains scons<decltype(params)>();
    // utils::AllPatterns pcons();


    // auto parser = scribe::BasicParser<decltype(tcons), decltype(scons), decltype(pcons)>(
    //     std::move(tcons), std::move(scons), std::move(pcons));
    // for (auto const afile : logfiles) {
    //     std::string buffer = ioutils::read<std::string>(afile.c_str());
    //     parser(buffer.begin(), buffer.end());
    // }
}
