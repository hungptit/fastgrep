#pragma once

#pragma once
#include "ioutils/fdwriter.hpp"
#include "utils.hpp"
#include "utils/memchr.hpp"
#include <cstring>
#include <string>
#include "colors.hpp"

namespace fastgrep {
    // Note: Stream means we read data by chunks and do not know when it will be ended.
    template <typename Matcher> class StreamPolicy {
      public:
        template <typename Params>
        StreamPolicy(const std::string &patt, Params &&params)
            : matcher(patt, params.regex_mode),
              lines(1),
              pos(0),
              linebuf(),
              color(params.color()),
              linenum(params.linenum()),
              console(ioutils::StreamWriter::STDOUT) {}

        ~StreamPolicy() {
            if (color) { console.write(RESET_COLOR.data(), RESET_COLOR.size()); }
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

                // Skip empty lines if we can
                while (ptr < end && (*ptr == EOL)) {
                    ++ptr;
                    ++lines;
                }
                start = ptr;

                // Stop if we reach the end of the buffer.
                if (start == end) break;
            }

            // Update the line buffer with leftover data.
            if (start < end) { linebuf.append(start, end - start); }
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
        std::string file_name;

        void process_line(const char *begin, const size_t len) {
            if (matcher.is_matched(begin, len)) {
                const size_t buflen = len - 1;
                if (!linenum) {
                    if (!color) {
                        if (!file_name.empty()) {
                            console.write(file_name.data(), file_name.size());
                            console.put(':');
                        }
                        console.write(begin, buflen);
                        console.eol();
                    } else {
                        if (!file_name.empty()) {
                            console.write(BOLD_BLUE.data(), BOLD_BLUE.size());
                            console.write(file_name.data(), file_name.size());
                            console.put(':');
                        }
                        console.write(BOLD_GREEN.data(), BOLD_GREEN.size());
                        console.write(begin, buflen);
                        console.eol();
                    }
                } else {
                    std::string numstr = std::to_string(lines);
                    if (!color) {
                        if (!file_name.empty()) {
                            console.write(file_name.data(), file_name.size());
                            console.put(':');
                        }
                        console.write(numstr.data(), numstr.size());
                        console.put(':');
                        console.write(begin, buflen);
                        console.eol();
                    } else {
                        if (!file_name.empty()) {
                            console.write(BOLD_BLUE.data(), BOLD_BLUE.size());
                            console.write(file_name.data(), file_name.size());
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
        void set_filename(const char *fname) { file_name = std::string(fname); }

        // Process text data in the linebuf.
        void finalize() {
            process_line(linebuf.data(), linebuf.size());
            linebuf.clear();
            lines = 1;
            pos = 0;
        }
    };
} // namespace fastgrep
