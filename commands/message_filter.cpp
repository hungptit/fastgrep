#include "fmt/format.h"
#include <iostream>
#include <string>

#include "algorithms.hpp"
#include "message_filter.hpp"

#include "boost/program_options.hpp"

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    std::vector<std::string> log_files;
    po::options_description desc("Allowed options");
    std::string start_time, stop_time;
    std::string pattern;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
		("verbose,v", "Display verbose information.")
		("info,i", "Display information messages.")
		("error,e", "Display error messages.")
		("start-time,t", po::value<std::string>(&start_time), "Start time in 'yyyy-mm-dd hh::mm::ss' format.")
		("stop-time,s", po::value<std::string>(&stop_time), "Stop time in 'yyyy/mm/dd hh::mm::ss' format")
		("pattern,p", po::value<std::string>(&pattern), "Search pattern")
        ("log-files,l", po::value<std::vector<std::string>>(&log_files), "Scribe log files")
        ("output,o", po::value<std::vector<std::string>>(&log_files), "Output file");
    // clang-format on

    // Parse input arguments
    po::positional_options_description p;
    p.add("log-files", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: message_filter [options]\n";
        std::cout << desc;
        std::cout << "\nExamples:\n";
        return EXIT_SUCCESS;
    }

    // Init input parameters
    scribe::FilterParams params(start_time, stop_time, pattern);

    if (vm.count("verbose")) params.print();

    // // Construct time constraints
    // scribe::ScribeHeaderTimeConstraints time_constraints(params.start, params.stop);

    // // Construct search constraints
    // scribe::ScribeMessagePattern search_patterns(params.pattern);

    // Search for desired patterns in a list of log files.
	using String = std::string;
	using Patterns = scribe::Patterns<String>;
    using MessageFilter = typename scribe::MessageFilter<Patterns, String>;
	// scribe::AllMessages all;
	Patterns patt(pattern);
	MessageFilter filter(std::move(patt));
    scribe::FileReader<1 << 16, MessageFilter> message_filter;
    for (auto afile : log_files) { message_filter(afile.c_str(), filter); }
}
