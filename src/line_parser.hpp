#pragma once

#include "fmt/format.h"
#include "unistd.h"
#include <limits>
#include "utils/memchr.hpp"

namespace scribe {
    // A simple parser which computes the file size, the number of lines, and
    // the maximum/minimum length of lines.
    class LineStats {
      public:
        void operator()(const char *begin, const char *end) {
            // Count the number of lines
            const char *ptr = begin;
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
                // Update the line counter
                ++lines;

                // Update the max and min line length.
                const size_t new_eol = file_size + ptr - begin;
                const size_t len = new_eol - current_eol - 1;
                max_len = len > max_len ? len : max_len;
                min_len = len < min_len ? len : min_len;
                current_eol = new_eol;

                // Move to the next character.
                ++ptr;
            }

            // Update the current position of the read buffer.
            file_size += end - begin;
        }

        void print(const std::string &title) const {
            fmt::print("Number of bytes: {}\n", file_size);
            fmt::print("Number of lines: {}\n", lines);
            fmt::print("Max line length: {}\n", max_len);
            fmt::print("Min line lenght: {}\n", min_len);
            fmt::print("File size: {}\n", file_size);
        }

        size_t file_size = 0;
        size_t lines = 0;
        size_t max_len = std::numeric_limits<size_t>::min();
        size_t min_len = std::numeric_limits<size_t>::max();
        size_t current_eol = 0;

      private:
        static constexpr char EOL = '\n';
    };
} // namespace scribe
