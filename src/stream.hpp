#pragma once

#include "constants.hpp"
#include "ioutils/fdwriter.hpp"
#include "output.hpp"
#include "utils.hpp"
#include "utils/memchr.hpp"
#include <cstring>
#include <string>

namespace fastgrep {
    // Reference: https://misc.flogisoft.com/bash/tip_colors_and_formatting
    static const std::string BOLD_GREEN = "\033[1;32m"; // Normal, Green
    static const std::string BOLD_BLUE = "\033[1;34m";  // Bold, Blue
    static const std::string BOLD_WHITE = "\033[1;97m"; // Normal, white
    static const std::string RESET_COLOR = "\033[0m";   // Reset

    // Note: Stream means we read data by chunks and do not know when it will be ended.
    template <typename Matcher> class StreamPolicy {
      public:
        template <typename Params>
        StreamPolicy(const std::string &patt, Params &&params)
            : matcher(patt, params.regex_mode), lines(1), pos(0), linebuf(), color(params.color()),
              linenum(params.linenum()), console(ioutils::StreamWriter::STDOUT) {}

        ~StreamPolicy() {
            if (color) {
                console.write(RESET_COLOR.data(), RESET_COLOR.size());
            }
        }

        void process(const char *begin, const size_t len) {
            const char *start = begin;
            const char *end = begin + len;
            const char *ptr = begin;
#ifdef USE_AVX2
            while ((ptr = utils::avx2::memchr(ptr, EOL, end - ptr))) {
#elif
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
#endif
                if (linebuf.empty()) {
                    process_line(start, ptr - start + 1);
                } else {
                    linebuf.append(start, ptr - start + 1);
                    process_line(linebuf.data(), linebuf.size());
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

      protected:
        Matcher matcher;
        size_t lines = 1;
        size_t pos = 0;
        std::string linebuf;
        bool color = false;
        bool linenum = false;
        ioutils::StreamWriter console;
        const char *file = nullptr;
        int len = 0;

        void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                const size_t buflen = len - 1;
                if (!linenum) {
                    if (!color) {
                        if (len > 0) {
                            console.write(file, len);
                            console.put(':');
                        }
                        console.write(begin, buflen);
                        console.eol();
                    } else {
                        if (file) {
                            console.write(BOLD_BLUE.data(), BOLD_BLUE.size());
                            console.write(file, len);
                            console.put(':');
                        }
                        console.write(BOLD_GREEN.data(), BOLD_GREEN.size());
                        console.write(begin, buflen);
                        console.eol();
                    }
                } else {
                    std::string numstr = std::to_string(lines);
                    if (!color) {
                        console.write(file, len);
                        console.put(':');
                        console.write(numstr.data(), numstr.size());
                        console.put(':');
                        console.write(begin, buflen);
                        console.eol();
                    } else {
                        if (file) {
                            console.write(BOLD_BLUE.data(), BOLD_BLUE.size());
                            console.write(file, len);
                            console.put(':');
                        }
                        console.write(BOLD_WHITE.data(), BOLD_WHITE.size());
                        console.write(numstr.data(), numstr.size());
                        console.put(':');
                        console.write(BOLD_GREEN.data(), BOLD_GREEN.size());
                        console.write(begin, buflen);
                        console.eol();
                    }
                }
            }
        }

        // Set the file name so we can display our results better.
        void set_filename(const char *fname) {
            file = fname;
            len = strlen(file);
        }

        // Process text data in the linebuf.
        void finalize() {
            process_line(linebuf.data(), linebuf.size());
            linebuf.clear();
            lines = 1;
            pos = 0;
        }
    };
} // namespace fastgrep
