#include "fmt/format.h"
#include <string>

#include "ioutils/ioutils.hpp"

#include "scribe.hpp"
#include "parser.hpp"
#include "constraints.hpp"

#include <time.h>

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

const std::string datafile("data.log");

#include "scribe.hpp"
#include "header_parser.hpp"
#include "body_parser.hpp"


TEST_CASE("Control messages", "") {
    std::string buffer = ioutils::read<std::string>(datafile.c_str());
<<<<<<< HEAD
=======
    using Parser = scribe::BasicParser<scribe::TimeAll, scribe::AllPatterns, scribe::AllPatterns>;
    // auto parser = Parser();
    // parser(buffer.begin(), buffer.end());
>>>>>>> 7c8a11201d4bb608e502f4056a93b905c30fdbf0
}
