#include "fmt/format.h"
#include <string>

#include "ioutils.hpp"

#include "constraints.hpp"
#include "parser.hpp"
#include "parser_array.hpp"
#include "scribe.hpp"
#include "timer.hpp"
#include "wordcount.hpp"
#include "producer.hpp"
#include <time.h>

int main(int argc, char *argv[]) {
    scribe::MessageFilter<1 << 16> filter;
    utils::ElapsedTime<utils::SECOND> timer;
    // for (auto idx = 1; idx < argc; ++idx) {
    //     fmt::print("lines: {0}\n", filter(argv[idx]));
    // }

	std::string data1("[02/06/2018 16:46:07 job177.athenahealth.com generic.pdq_outbound 58653] {This is the first messsage}\n");
	std::string data2("[02/06/2018 16:46:07 job177.athenahealth.com generic.pdq_outbound 58653] {This is the second messsage}\n");
	std::string data3("[02/06/2018 16:46:07 job177.athenahealth.com generic.pdq_outbound 58653] {This is the third messsage}\n");
	filter.parse(data1 + data2 + data3);
}
