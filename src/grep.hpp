#pragma once

#include <cstring>
#include <string>
#include "fmt/format.h"
#include "utils/regex_matchers.hpp"

namespace fastgrep {
    static constexpr char EOL = '\n';

    class GrepPolicy {
      public:
        GrepPolicy(const std::string &patt) : matcher(patt) {}

        ~GrepPolicy() { process_linebuf(); }

        void process(const char *begin, const size_t len) {
            const char *start = begin;
            const char *end = begin + len;
            const char *ptr = begin;
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
                linebuf.append(start, ptr - start + 1);
                process_linebuf();
                linebuf.clear();

                // Update parameters
                start = ++ptr;
                ++lines;

                // Stop if we reach the end of the buffer.
                if (start == end) break;
            }

            // Update the line buffer with leftover data.
            if (start != end) { linebuf.append(start, end - start); }
            pos += len;
        }

        size_t lines = 0;
        size_t pos = 0;
        utils::hyperscan::RegexMatcher matcher;

      protected:
        void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                fmt::print("{}\n", std::string(begin, begin + len));
            }
        }

        void process_linebuf() {
            if (matcher.is_matched(linebuf.data(), linebuf.size())) {
                fmt::print("{}\n", linebuf);
            }
        }

        std::string linebuf;
    };

    class GrepPolicyNew {
      public:
        GrepPolicyNew(const std::string &patt) : matcher(patt) {}

        ~GrepPolicyNew() { process_linebuf(); }

        void process(const char *begin, const size_t len) {
            const char *start = begin;
            const char *end = begin + len;
            const char *ptr = begin;
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
                if (linebuf.empty()) {
                    process_line(start, ptr - start);
                } else {
                    linebuf.append(start, ptr - start + 1);
                    process_linebuf();
                    linebuf.clear();
                }

                // Update parameters
                start = ++ptr;
                ++lines;

                // Stop if we reach the end of the buffer.
                if (start == end) break;
            }

            // Update the line buffer with leftover data.
            if (start != end) { linebuf.append(start, end - start); }
            pos += len;
        }

        size_t lines = 0;
        size_t pos = 0;
        std::string linebuf;
        utils::hyperscan::RegexMatcher matcher;

      protected:
        void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                fmt::print("{}\n", std::string(begin, begin + len));
            }
        }

        void process_linebuf() {
            if (matcher.is_matched(linebuf.data(), linebuf.size())) {
                fmt::print("{}\n", linebuf);
            }
        }

    };

    struct ExactMatchPolicy {};
} // namespace fastgrep
