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
    scribe::MessageFilterParams params;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
		("verbose,v", "Display verbose information.")
		("info,i", "Display information messages.")
		("error,e", "Display error messages.")
		("begin,t", po::value<std::string>(&start_time), "Start time in 'yyyy-mm-dd hh::mm::ss' format.")
		("end,s", po::value<std::string>(&stop_time), "Stop time in 'yyyy-mm-dd hh::mm::ss' format")
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
    if (vm.count("verbose")) params.print();

    // // Construct time constraints
    // scribe::ScribeHeaderTimeConstraints time_constraints(params.start, params.stop);

    // Search for desired patterns in a list of log files.
	// using Patterns = utils::baseline::Contains;
	using Patterns = utils::sse2::Contains;
	// using Patterns = utils::avx2::Contains;
    using MessageFilter = typename scribe::MessageFilter<Patterns>;
	MessageFilter filter(pattern);
    scribe::FileReader<1 << 16, MessageFilter> reader;
    for (auto afile : log_files) { reader(afile.c_str(), filter); }
}
