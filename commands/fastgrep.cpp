#include "fmt/format.h"
#include <iostream>
#include <string>

#include "algorithms.hpp"
#include "boost/program_options.hpp"
#include "message_filter.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/regex_matchers.hpp"
#include "utils/timestamp.hpp"

namespace {
    auto parse_timestamp_value(const std::string &timestr, utils::Timestamp default_value) {
        constexpr size_t TIMESTAMP_LENGTH = 19;
        if (timestr.empty()) {
            return default_value;
        } else {
            if (timestr.size() == TIMESTAMP_LENGTH) {
                return utils::parse_timestamp<utils::Timestamp>(timestr.data());
            } else {
                throw std::runtime_error(std::string("Invalid time string: ") + timestr);
            }
        }
    }

    template <typename Constraints> void filter(const scribe::MessageFilterParams &params) {
        constexpr size_t BUFFER_SIZE = 1 << 16;
        Constraints cons(params);
        using MessageFilter = typename scribe::MessageFilter<Constraints>;
        MessageFilter filter(params);
        scribe::FileReader<BUFFER_SIZE, MessageFilter> reader;
        for (auto afile : params.infiles) { reader(afile.c_str(), filter); }
    }

    template <typename T> void exec(const scribe::MessageFilterParams &params) {
        const int case_number = ((!params.pattern.empty()) << 2) +
                                ((params.begin != utils::MIN_TIME) << 1) +
                                (params.end != utils::MAX_TIME);
        // fmt::print("case_number: {}\n", case_number);
        switch (case_number) {
        case 0:
            filter<scribe::All>(params);
            break;
        case 4:
            filter<typename scribe::SimpleConstraints<T>>(params);
            break;
        default:
            filter<typename scribe::BasicConstraints<T>>(params);
            break;
        }
    }
} // namespace

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string begin_time, end_time;
    scribe::MessageFilterParams params;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
		("verbose,v", "Display verbose information.")
		("info", "Display information messages.")
		("error", "Display error messages.")
		("no-regex", "Do not use regex engine for pattern matching.")
		("begin,b", po::value<std::string>(&begin_time), "Begin time in 'mm-dd-yyyy hh:mm:ss' format.")
		("end,e", po::value<std::string>(&end_time), "End time in 'mm-dd-yyyy hh:mm:ss' format")
		("pattern,p", po::value<std::string>(&params.pattern), "Search pattern")
        ("log-files,l", po::value<std::vector<std::string>>(&params.infiles), "Scribe log files")
        ("output,o", po::value<std::string>(&params.outfile), "Output file");
    // clang-format on

    // Parse input arguments
    po::positional_options_description p;
    p.add("log-files", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: fastgrep [options] pattern data.log\n";
        std::cout << desc;
        std::cout << "\nExamples:\n";
        std::cout
            << "\tfastgrep --begin \"04-02-2018 00:31:00\" --end \"04-02-2018 00:31:16\" "
               "-p '\"LEVEL\":\"error\"' "
               "/mnt/weblogs/scribe/workqueue-execution/workqueue-execution-2018-04-02_00000\n";
        return EXIT_SUCCESS;
    }

    // Process input parameters
    params.begin = parse_timestamp_value(begin_time, utils::MIN_TIME);
    params.end = parse_timestamp_value(end_time, utils::MAX_TIME);

    // Display input arguments in verbose mode.
    if (vm.count("verbose")) scribe::print_filter_params(params);

    // Search for desired lines from given log files.
    if (vm.count("no-regex")) {
        exec<utils::avx2::Contains>(params);
    } else {
        exec<utils::hyperscan::RegexMatcher>(params);
    }

	// Return
	return EXIT_SUCCESS;
}
