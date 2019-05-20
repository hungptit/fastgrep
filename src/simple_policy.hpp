#pragma once

#include "constants.hpp"
#include "output.hpp"
#include "utils.hpp"
#include <string>

namespace fastgrep {
    // Note: We only use memmap reader if file content can be mapped entirely in memory. This assumption
    // means all lines are belong the grep range.

    // The matcher will take the regular expression pattern and a regex mode. This should be enough for any
    // text searching task.
    template <typename Matcher> class SimplePolicy {
      public:
        template <typename Params>
        SimplePolicy(const std::string &patt, Params &&params)
            : matcher(patt, params.regex_mode),
              lines(1),
              pos(0),
              console(),
              linenum(params.linenum()),
              color(params.color()) {}

        void process(const char *begin, const size_t len) {
            const char *start = begin;
            const char *end = begin + len;
            const char *ptr = begin;
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
                process_line(start, ptr - start + 1);

                // Skip empty lines if we can
                while (ptr < end && (*ptr == EOL)) {
                    ++ptr;
                    ++lines;
                }
                start = ptr;
            }

            // Update the line buffer with leftover data.
            if (start < end) { process_line(start, end - start); }
            pos += len;
        }

        void reset() {
            lines = 0;
            pos = 0;
        }

      protected:
        Matcher matcher;
        size_t lines = 1;
        size_t pos = 0;

        bool linenum;
        bool color;
        const char *file;

        // Set the file name so we can display our results better.
        void set_filename(const char *fname) { file = fname; }

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
    };
} // namespace fastgrep
