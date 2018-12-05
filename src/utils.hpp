#pragma once

#include "fmt/format.h"
#include <string>

namespace fastgrep {
    // TODO: How can we avoid memory copy.
    void print_color_text(const char *begin, const char *end, const size_t linenum) {
        fmt::print("\033[1;32m{0}:\033[1;39m{1}\033[0m\n", std::string(begin, end - begin), linenum);
    }

    void print_plain_text(const char *begin, const char *end, const size_t linenum) {
        fmt::print("{0}:{1}\n", std::string(begin, end - begin), linenum);
    }
    void print_color_text(const char *begin, const char *end) {
        fmt::print("\033[1;32m{0}\033[0m\n", std::string(begin, end - begin));
    }

    void print_plain_text(const char *begin, const char *end) {
        fmt::print("{0}\n", std::string(begin, end - begin));
    }
}
