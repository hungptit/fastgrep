#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "scribe.hpp"

namespace scribe {
    // Parse timestamp.
    class ScribeTimestampParser {
      public:
        ScribeTimestampParser() noexcept {
            tm.tm_isdst = 0; // We skip day light time saving for now.
        }

        // Timestamp has this format mm/dd/yyyy HH:MM:SS
        std::time_t operator()(const char *begin) {
            const char *ptr = begin;

            tm.tm_mon = parse_two_digits(ptr) - SHIFT_MONTH;
            ptr += 3;

            tm.tm_mday = parse_two_digits(ptr);
            ptr += 3;

            tm.tm_year = parse_four_digits(ptr) - SHIFT_YEAR;

            ptr += 5;
            tm.tm_hour = parse_two_digits(ptr);

            ptr += 3;
            tm.tm_min = parse_two_digits(ptr);

            ptr += 3;
            tm.tm_sec = parse_two_digits(ptr);

            tm.tm_isdst = 0;
            return mktime(&tm);
        }

        static int parse_two_digits(const char *begin) {
            const char *ptr = begin;
            return ptr[0] * 10 + ptr[1] - SHIFT_2;
        }

        static int parse_four_digits(const char *begin) {
            const char *ptr = begin;
            return ptr[0] * 1000 + ptr[1] * 100 + ptr[2] * 10 + ptr[3] - SHIFT_4;
        }

      private:
        struct tm tm;
        static constexpr char ZERO = '0';
        static constexpr int SHIFT_2 = ZERO * 11;
        static constexpr int SHIFT_4 = ZERO * 1111;
        static constexpr int SHIFT_YEAR = 1900;
        static constexpr int SHIFT_MONTH = 1;
    };

    // Parser the scribe message header.
    class MessageHeaderParser {
      public:
        MessageHeader operator()(const char *begin, const char *end) {
            assert((begin + 32) < end);
            const char *ptr = begin + 1;
            MessageHeader header;
            header.timestamp = time_parser(ptr);
            ptr += 20;

            // Parse server name
            const char *pos = static_cast<const char *>(memchr(ptr, SPACE, end - ptr));
            assert(pos != NULL);
            header.server.append(ptr, pos - ptr);

            // Parse pool name.
            ptr = pos + 1;
            assert(ptr < end);
            pos = static_cast<const char *>(memchr(ptr, SPACE, end - ptr));
            assert(pos != NULL);
            header.pool.append(ptr, pos - ptr);

            // Parse process id.
            ptr = pos + 1;
            assert(ptr < end);
            header.pid = atoi(ptr);

            return header;
        }

      private:
        ScribeTimestampParser time_parser;
        static constexpr char SPACE = ' ';
    };
} // namespace scribe
