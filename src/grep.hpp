#pragma once

#include "fmt/format.h"
#include <cstring>
#include <string>

#include "utils/memchr.hpp"

namespace fastgrep {
    static constexpr char EOL = '\n';
    template <typename Matcher> class GrepPolicy {
      public:
        GrepPolicy(const std::string &patt) : matcher(patt) {}

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
        size_t lines = 0;
        size_t pos = 0;
        std::string linebuf;

      protected:
        void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                fmt::print("{0}:{1}", lines, std::string(begin, len));
            }
        }

        void process_linebuf() { process_line(linebuf.data(), linebuf.size()); }
    };

    namespace experiments {
        template <typename Matcher> class GrepPolicy {
          public:
            GrepPolicy(const std::string &patt) : matcher(patt) {}

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
                if (start != end) {
                    linebuf.append(start, end - start);
                    process_linebuf();
                }
                pos += len;
            }

            Matcher matcher;
            std::string linebuf;
            size_t lines = 0;
            size_t pos = 0;

          protected:
            void process_linebuf() {
                if (matcher.is_matched(linebuf)) { fmt::print("{0}:{1}", lines, linebuf); }
            }
        };

    } // namespace experiments
} // namespace fastgrep
