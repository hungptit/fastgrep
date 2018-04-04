#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "hs/hs.h"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/memchr.hpp"
#include "utils/timestamp.hpp"
#include "utils/timeutils.hpp"

namespace scribe {
    struct MessageFilterParams {
        utils::Timestamp begin;
        utils::Timestamp end;
        std::string server;
        std::string pool;
        std::string pattern;
        std::vector<std::string> infiles;
        std::string outfile;

        MessageFilterParams()
            : begin(), end(), server(), pool(), pattern(), infiles(), outfile() {}
    };

    void print_filter_params(const MessageFilterParams &params) {
        utils::TimePrinter time_printer("%m-%d-%Y %H:%M:%S");
        fmt::print("Begin time: {}\n", time_printer(params.begin.to_tm()));
        fmt::print("End time: {}\n", time_printer(params.end.to_tm()));
        if (!params.server.empty()) fmt::print("Server name: \n", params.server);
        if (!params.pool.empty()) fmt::print("Pool name: {}\n", params.pool);
        if (!params.pattern.empty()) fmt::print("Search pattern: {}\n", params.pattern);
    }

    // No constraint.
    struct All {
        All(const MessageFilterParams &) {}
        bool operator()(const std::string &line) { return true; }
    };

    // A simple pattern constraint
    template <typename T> class SimpleConstraints {
      public:
        using pattern_type = T;
        explicit SimpleConstraints(const MessageFilterParams &params)
            : contains(params.pattern) {}
        SimpleConstraints(const SimpleConstraints &obj) = delete;
        bool operator()(const std::string &line) { return contains(line); }

      private:
        pattern_type contains; // Search for a given string pattern
    };

    // A pattern + timestamp constraint.
    template <typename T> class BasicConstraints {
      public:
        using pattern_type = T;
        using time_type = utils::Timestamp;
        using time_constraint_type = typename utils::Between<time_type>;

        BasicConstraints(const MessageFilterParams &params)
            : contains(params.pattern), between(params.begin, params.end) {}
        bool operator()(const std::string &line) {
            if (line.size() < 20) return false; // Skip invalid line
            // Check the time constraints fist since it is cheaper than pattern constraint.
            auto const t = utils::parse_timestamp<time_type>(line.data() + 1);
            if (!between(t)) return false;
            return contains(line);
        }

      private:
        pattern_type contains; // Search for a given string pattern
        time_constraint_type between;
    };

    // Filter message that match given constraints.
    template <typename Constraints> class MessageFilter {
      public:
        explicit MessageFilter(const MessageFilterParams &params)
            : buffer(), lines(0), constraints(params) {
            buffer.reserve(1 << 12);
        }

        explicit MessageFilter(const MessageFilter &value) =
            delete; // We do not support copy constructor.

        ~MessageFilter() {
            if (!buffer.empty()) process();
        }

        void operator()(const char *begin, const char *end) {
            const char *start = begin;
            const char *ptr = begin;

            // Parse line by line
            while ((ptr = static_cast<const char *>(memchr_avx2(ptr, EOL, end - ptr)))) {
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
