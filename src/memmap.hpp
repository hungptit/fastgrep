#pragma once

#include "constants.hpp"
#include "utils.hpp"
#include <string>

namespace fastgrep {
    // Note: We only use memmap reader if file content can be mapped entirely in memory. This assumption
    // means all lines are belong the grep range.

    // The matcher will take the regular expression pattern and a regex mode. This should be enough for any
    // text searching task.
    template <typename Matcher> class MMapPolicy {
      public:
        template <typename Params>
        MMapPolicy(const std::string &patt, Params &&params) : matcher(patt, params.regex_mode) {
            linenum = params.linenum();
            color = params.color();
        }

        void process(const char *begin, const size_t len) {
            const char *start = begin;
            const char *end = begin + len;
            const char *ptr = begin;
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
                process_line(start, ptr - start + 1);

                // Update parameters
                start = ++ptr;
                ++lines;

                // Stop if we reach the end of the buffer.
                if (start == end) break;
            }

            // Update the line buffer with leftover data.
            if (start != end) { process_line(start, end - start); }
            pos += len;
        }

        void reset() {
            lines = 0;
            pos = 0;
        }

        Matcher matcher;
        size_t lines = 1;
        size_t pos = 0;

        bool linenum;
        bool color;

      protected:
        void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                const size_t buflen = len - 1;
                if (linenum) {
                    if (!color) {
                        print_plain_text(begin, begin + buflen, lines);
                    } else {
                        print_color_text(begin, begin + buflen, lines);
                    }
                } else {
                    if (!color) {
                        print_plain_text(begin, begin + buflen);
                    } else {
                        print_color_text(begin, begin + buflen);
                    }
                }
            }
        }
    };
} // namespace fastgrep
