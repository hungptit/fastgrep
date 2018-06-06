s#include "fmt/format.h"
#include <string>

#include "ioutils/ioutils.hpp"

#include "algorithms.hpp"
#include "constraints.hpp"
#include "parser.hpp"
#include "parser_array.hpp"
#include "scribe.hpp"
#include "utils/timer.hpp"
#include "line_parser.hpp"
#include <time.h>

int main(int argc, char *argv[]) {
    scribe::FileReader<1 << 16, scribe::LineStats> parser;
    scribe::LineStats stats;
    utils::ElapsedTime<utils::SECOND> timer;
    for (auto idx = 1; idx < argc; ++idx) { parser(argv[idx], stats); }
    stats.print("Summary:");
}
