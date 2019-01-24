#pragma once

#include "constants.hpp"
#include "fmt/format.h"
#include "output.hpp"
#include "utils.hpp"
#include "utils/memchr.hpp"
#include <cstring>
#include <string>

namespace fastgrep {
    // Note: Stream means data are read by chunks and we do not know when it will be ended.
    template <typename Matcher, typename Console = FMTPolicy> class StreamPolicy {
      public:
        template <typename Params>
        StreamPolicy(const std::string &patt, Params &&params)
            : matcher(patt, params.regex_mode), lines(1), pos(0), linebuf(), console(),
              color(params.color()), linenum(params.linenum()) {}

        // We do need to process the last line at the end.
        ~StreamPolicy() { process_linebuf(); }

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
                if (ptr >= end) break;
            }

            // Update the line buffer with leftover data.
            if (ptr == nullptr) { linebuf.append(start, end - start); }
            pos += len;
        }

        Matcher matcher;
        size_t lines = 1;
        size_t pos = 0;
        std::string linebuf;
        Console console;
        bool color = false;
        bool linenum = false;

      protected:
        virtual void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                const size_t buflen = len - 1;
                if (!linenum) {
                    if (!color) {
                        console.print_plain_text(begin, begin + buflen);
                    } else {
                        console.print_color_text(begin, begin + buflen);
                    }
                } else {
                    if (!color) {
                        console.print_plain_text(begin, begin + buflen, lines);
                    } else {
                        console.print_color_text(begin, begin + buflen, lines);
                    }
                }
            }
        }

        // Process text data in the linebuf.
        void finalize() {
            process_line(linebuf.data(), linebuf.size());
            lines = 1;
            pos = 0;
        }
        void process_linebuf() { process_line(linebuf.data(), linebuf.size()); }
    };
} // namespace fastgrep
