#pragma once

#include "scribe.hpp"
#include <sstream>
#include <vector>

#include "folly/FBString.h"

namespace scribe {
    struct FilterParams {
        FilterParams(const std::string &start_time, const std::string &stop_time, const std::string &patt) {
            start = start_time.empty() ? 0 : parse(start_time);
            stop = stop_time.empty() ? 0 : parse(stop_time);
            pattern = patt;
        }

        std::time_t parse(const std::string &timestamp) {
            struct tm tm;
            const char *ts = &timestamp[0];
            strptime(ts, "%Y-%m-%d %H:%M:%S", &tm);
            tm.tm_isdst = 0; // TODO: Disable day light time saving for now.
            return mktime(&tm);
        }

        const char *get_timestamp(const std::time_t t) {
            struct tm *timestamp = localtime(&t);
            strftime(buffer, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", timestamp);
            return buffer;
        }

        void print() {
            if (start) fmt::print("Start time: {}\n", get_timestamp(start));
            if (stop) fmt::print("Stop time: {}\n", get_timestamp(stop));
            if (!pattern.empty()) fmt::print("Search pattern: {}\n", pattern);
        }

        std::time_t start;
        std::time_t stop;
        std::string pattern;

        // Temporary variables
        static size_t constexpr BUFFER_SIZE = 20;
        char buffer[20];
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

    // Pattern constraint.
    struct ScribeMessagePattern {
        using String = std::string;
        ScribeMessagePattern(const String &s) : pattern(s) {}
        bool operator()(const String &buffer) {
            if (pattern.empty()) return true;
            return buffer.find(pattern) != String::npos;
        }
        String pattern;
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

    struct AllMessages {
        template <typename String> bool operator()(const String &) { return true; }
    };

    // Search for a pattern.
    template <typename String> class Patterns {
      public:
        Patterns(const String &patt) : pattern(patt){};
        bool operator()(String &&buffer) { return buffer.find(pattern) != String::npos; }

      private:
        String pattern;
    };

    // Filter message that match given constraints.
    template <typename Constraint, typename String> class MessageFilter {
      public:
        // using String = std::string;
        // using String = folly::fbstring;
        MessageFilter(Constraint &&cons)
            : buffer(), lines(0), constraints(std::forward<Constraint>(cons)) {}
        MessageFilter(const MessageFilter &value) = delete; // We do not support copy constructor.
        ~MessageFilter() {
            if (!buffer.empty()) print();
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
                print();

                // Update start
                start = ++ptr;

                // Stop if we reach the end of buffer.
                if (start == end) break;
            }

            // Update line buffer with leftover data.
            if (start != end) { buffer.append(start, end - start); }
        }

      private:
        String buffer;
        size_t lines;
        Constraint constraints;

        void print() {
            if (constraints(std::move(buffer))) { fmt::print("{}", buffer.data()); }
            buffer.clear(); // Reset the buffer.
        }
    };
} // namespace scribe
