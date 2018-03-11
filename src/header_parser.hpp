#pragma once
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>

namespace scribe {
    // A scribe header has this format "[03/08/2018 12:00:00 node1234.example.com generic.workqueue 123456]"
    struct MessageHeader {
        static constexpr size_t SERVER_BUFFER_SIZE = 32;
        static constexpr size_t POOL_BUFFER_SIZE = 48;
        std::time_t timestamp;           // Timestamp: 03/08/2018 23::00:50
        char server[SERVER_BUFFER_SIZE]; // A server address: job1120.domain_name.com
        char pool[POOL_BUFFER_SIZE];     // A job pool name i.e job.pool.name
        int pid;                         // This is a process id
    };

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
            memcpy(header.server, ptr, std::min<size_t>(pos - ptr, MessageHeader::SERVER_BUFFER_SIZE));

            // Parse pool name.
            ptr = pos + 1;
            assert(ptr < end);
            pos = static_cast<const char *>(memchr(ptr, SPACE, end - ptr));
            assert(pos != NULL);
            memcpy(header.pool, ptr, std::min<size_t>(pos - ptr, MessageHeader::POOL_BUFFER_SIZE));

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
