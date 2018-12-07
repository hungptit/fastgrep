#pragma once

#include "fmt/format.h"
#include <cstring>
#include <string>
#include "utils/memchr.hpp"
#include "memmap.hpp"
#include "stream.hpp"

namespace fastgrep {

    // This policy assumes that lines are all inside the given buffer.

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
