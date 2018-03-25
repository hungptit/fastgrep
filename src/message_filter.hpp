#pragma once

#include <sstream>
#include <vector>

#include "constraints.hpp"
#include "header_parser.hpp"
#include "utils/matchers.hpp"
#include "utils/timeutils.hpp"

namespace scribe {
    struct MessageFilterParams {
        std::time_t begin_time;
        std::time_t end_time;
        std::string server;
        std::string pool;
        std::string pattern;
        MessageFilterParams() : begin_time(0), end_time(0), server(), pool(), pattern() {}
        void print() const {
            utils::TimePrinter time_printer("%Y-%m-%d %H:%M:%S");
            fmt::print("Begin time: {}\n", time_printer(begin_time));
            fmt::print("End time: {}\n", time_printer(end_time));
            if (!server.empty()) fmt::print("Server name: \n", server);
            if (!pool.empty()) fmt::print("Pool name: {}\n", pool);
            if (!pattern.empty()) fmt::print("Search pattern: {}\n", pattern);
        }
    };

    // Time constraints.
    struct ScribeHeaderTimeConstraints {
        ScribeHeaderTimeConstraints(const std::time_t begin, std::time_t end) : start(begin), stop(end) {}
        bool operator()(std::time_t t) {
            if ((start == 0) && (stop == 0)) return true;
            if (stop == 0) return t >= start;
            return (t >= start) && (t <= stop);
        }
        std::time_t start = 0;
        std::time_t stop = 0;
    };

    // Parse timestamp in the scribe header.
    struct ParseScribeTimestamp {
        std::time_t operator()(const char *begin, const char *end) {
            if (end < (begin + BUFFER_SIZE)) {
                fmt::MemoryWriter writer;
                writer << "Invalid timestamp string: " << std::string(begin, end - begin);
                throw(std::runtime_error(writer.str()));
            }
            memcpy(buffer, begin, BUFFER_SIZE);
            strptime(buffer, "%m/%d/%Y %H:%M:%S", &tm);
            tm.tm_isdst = 0; // TODO: Disable day light time saving for now.
            std::time_t t = mktime(&tm);
            // fmt::print("Timestamp: {}\n", get_timestamp(t));
            return t;
        }

        const char *get_timestamp(std::time_t t) {
            struct tm *timestamp = localtime(&t);
            strftime(buffer, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", timestamp);
            return buffer;
        }

        static size_t constexpr BUFFER_SIZE = 20;
        char buffer[20];
        struct tm tm;
    };

    // Filter message that match given constraints.
    template <typename Constraints> class MessageFilter {
      public:
        MessageFilter(const std::string &patt) : buffer(), lines(0), constraints(patt) {
            buffer.reserve(1 << 12);
        }
        MessageFilter(const MessageFilter &value) = delete; // We do not support copy constructor.
        ~MessageFilter() {
            if (!buffer.empty()) process();
        }

        void operator()(const char *begin, const char *end) {
            const char *start = begin;
            const char *ptr = begin;

            // Parse line by line
            while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
                buffer.append(start, ptr - start + 1);

                // Increase line counter
                ++lines;

                // Parse the data
				process();

                // Update start
                start = ++ptr;

                // Stop if we reach the end of buffer.
                if (start == end) break;
            }

            // Update line buffer with leftover data.
            if (start != end) { buffer.append(start, end - start); }
        }

      private:
        std::string buffer;
        size_t lines;
        Constraints constraints;
        static constexpr char EOL = '\n';
        void process() {
            if (constraints(buffer)) fmt::print("{}", buffer.data());
            buffer.clear(); // Reset the buffer.
        }
    };
} // namespace scribe
