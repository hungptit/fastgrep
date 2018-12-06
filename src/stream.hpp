#pragma once

#include "constants.hpp"
#include "fmt/format.h"
#include "utils.hpp"
#include "utils/memchr.hpp"
#include <cstring>
#include <string>

namespace fastgrep {
    // Note: Stream means data are read by chunks and we do not know when it will be ended.
    template <typename Matcher> class StreamPolicy {
      public:
        template <typename Params>
        StreamPolicy(const std::string &patt, Params &&params) : matcher(patt, params.regex_mode) {
            color = params.color();
            linenum = params.linenum();
            verbose = params.verbose();
        }

        void process(const char *begin, const size_t len) {
            const char *start = begin;
            const char *end = begin + len;
            const char *ptr = begin;
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
                if (linebuf.empty()) {
                    process_line(start, ptr - start + 1);
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
            if (start != end) {
                linebuf.append(start, end - start);
                process_line(linebuf.data(), linebuf.size());
            }
            pos += len;
        }

        Matcher matcher;
        size_t lines = 1;
        size_t pos = 0;
        bool verbose = false;
        bool color = false;
        bool linenum = false;
        std::string linebuf;

      protected:
        void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                const size_t buflen = len - 1;
                if (!linenum) {
                    if (!color) {
                        print_plain_text(begin, begin + buflen);
                    } else {
                        print_color_text(begin, begin + buflen);
                    }
                } else {
                    if (!color) {
                        print_plain_text(begin, begin + buflen, lines);
                    } else {
                        print_color_text(begin, begin + buflen, lines);
                    }
                }
            }
        }

        // Process text data in the linebuf.
        void process_linebuf() { process_line(linebuf.data(), linebuf.size()); }
    };
} // namespace fastgrep
