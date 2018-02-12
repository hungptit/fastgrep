#include "fmt/format.h"
#include <string>

#include "ioutils.hpp"

#include "constraints.hpp"
#include "parser.hpp"
#include "parser_array.hpp"
#include "scribe.hpp"
#include "timer.hpp"
#include "wordcount.hpp"
#include <time.h>

int main(int argc, char *argv[]) {
    scribe::WordCount<1 << 16> parser;
    utils::ElapsedTime<utils::SECOND> timer;
    for (auto idx = 1; idx < argc; ++idx) {
        auto stats = parser(argv[idx]);
        fmt::print("{0} => lines: {1}, bytes: {2}\n", argv[idx], stats.line_count, stats.byte_count);
    }
}
