#pragma once

#include <sstream>
#include <vector>

#include "constraints.hpp"
#include "header_parser.hpp"
#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
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

        MessageFilterParams() : begin(), end(), server(), pool(), pattern(), infiles(), outfile() {}
        void print() const {
            utils::TimePrinter time_printer("%m-%d-%Y %H:%M:%S");
            fmt::print("Begin time: {}\n", time_printer(begin.to_tm()));
            fmt::print("End time: {}\n", time_printer(end.to_tm()));
            if (!server.empty()) fmt::print("Server name: \n", server);
            if (!pool.empty()) fmt::print("Pool name: {}\n", pool);
            if (!pattern.empty()) fmt::print("Search pattern: {}\n", pattern);
        }
    };

    struct All {
        All(const MessageFilterParams &){};
        bool operator()(const std::string &line) { return true; }
    };

    class SimpleConstraints {
      public:
        using Contains = utils::avx2::Contains;
        SimpleConstraints(const MessageFilterParams &params) : contains(params.pattern) {}
        bool operator()(const std::string &line) { return contains(line); }

      private:
        Contains contains; // Search for a given string pattern
    };

    class BasicConstraints {
      public:
        using pattern_type = utils::avx2::Contains;
        using time_type = utils::Timestamp;
        using time_constraint_type = typename utils::Between<time_type>;
        BasicConstraints(const MessageFilterParams &params)
            : contains(params.pattern), between(params.begin, params.end) {}
        bool operator()(const std::string &line) {
            if (line.size() < 20) return false; // Skip invalid line
            // Check the time constraints since it is cheaper than pattern constraint.
            auto const t = utils::parse_timestamp<time_type>(line.data() + 1);
            return between(t) && contains(line);
        }

      private:
        pattern_type contains; // Search for a given string pattern
        time_constraint_type between;
    };

    // Filter message that match given constraints.
    template <typename Constraints> class MessageFilter {
      public:
        MessageFilter(const MessageFilterParams &params) : buffer(), lines(0), constraints(params) {
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
