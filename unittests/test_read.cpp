#include "fmt/format.h"
#include <string>

#include "ioutils.hpp"

#include "constraints.hpp"
#include "parser.hpp"
#include "parser_array.hpp"
#include "scribe.hpp"
#include "timer.hpp"
#include <time.h>

int main(int argc, char *argv[]) {
    scribe::WordCount parser;
    utils::ElapsedTime<utils::SECOND> timer;
    for (auto idx = 1; idx < argc; ++idx) {
        auto stats = parser(argv[idx]);
        fmt::print("lines: {0}, words: {1}, bytes: {2}\n", stats.line_count,
                   stats.word_count, stats.byte_count);
    }
}
