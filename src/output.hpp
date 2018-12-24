#pragma once

#include "fmt/format.h"
#include <unistd.h>
#include <sys/uio.h>

namespace fastgrep {
    struct FMTPolicy {
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
    };

    struct DirectPolicy {
        void print_color_text(const char *begin, const char *end, const size_t linenum) {
            // TODO: using write v for this task.
            fmt::print("\033[1;32m{0}:\033[1;39m{1}\033[0m\n", std::string(begin, end - begin), linenum);
        }

        void print_plain_text(const char *begin, const char *end, const size_t linenum) {
            // TODO: using write v for this task.
            ::write(STDIN_FILENO, begin, end - begin);
        }
        void print_color_text(const char *begin, const char *end) {
            // TODO: using write v for this task.
            fmt::print("\033[1;32m{0}\033[0m\n", std::string(begin, end - begin));
        }

        void print_plain_text(const char *begin, const char *end) {
            ::write(STDIN_FILENO, begin, end - begin);
        }        
    };
    
    struct StorePolicy {
        void print_color_text(const char *begin, const char *end, const size_t linenum) {
            lines.emplace_back(std::string(begin, end - begin));
            linenums.push_back(linenum);
        }

        void print_plain_text(const char *begin, const char *end, const size_t linenum) {
            lines.emplace_back(std::string(begin, end - begin));
            linenums.push_back(linenum);
        }
        void print_color_text(const char *begin, const char *end) {
            lines.emplace_back(std::string(begin, end - begin));
        }

        void print_plain_text(const char *begin, const char *end) {
            lines.emplace_back(std::string(begin, end - begin));
        }

        std::vector<std::string> lines{};
        std::vector<size_t> linenums;
    };
}
