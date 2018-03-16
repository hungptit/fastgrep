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
}
