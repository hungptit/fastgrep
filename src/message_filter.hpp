#pragma once

#include "scribe.hpp"
#include <sstream>
#include <vector>

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

    class MessageFilter {
      public:
        using HeaderConstraint = ScribeHeaderTimeConstraints;
        using MessageConstraint = ScribeMessagePattern;
        MessageFilter() = default;
        MessageFilter(const HeaderConstraint &c1, const MessageConstraint &c2)
            : time_constraint(c1), search_pattern(c2) {}
        MessageFilter(const MessageFilter &value) = delete; // We do not support copy constructor.
        ~MessageFilter() {
            // Need to parse leftover data.
			if (search_pattern(buffer) && buffer.size() > 20) {
				const char *begin = &buffer[0];
				parse(begin, begin + buffer.size() - 1);
			}
        }

        void operator()(const char *begin, const char *end) {
			char *start = begin;
			const char *ptr = begin;

			// Parse line by line
			while ((ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr)))) {
				buffer.append(start, ptr - start + 1);
				
				// Increase line counter
				++lines;
				
				// Parse the data
				fmt::print("{0}: {1}", lines, buffer);

				// Reset the line buffer
				buffer.clear();
				if ((ptr + 1) != end) {
					
				}
			}
			
			
            const char *ptr = static_cast<const char *>(memrchr(begin, EOL, end - begin));
            if (ptr && ((buffer.size() + ptr) > (begin + MAX_SIZE))) {
                // Filter messages based on user's constraints such as time,
                // string pattern, and server name etc
                buffer.append(begin, ptr - begin);

				// Only parse the buffer if the search pattern is found.
				if (search_pattern(buffer)) {
                    const char *begin = &buffer[0];
                    parse(begin, begin + buffer.size() - 1);
                }

                buffer.clear();
                buffer.append(ptr, end - ptr);
            }

            // This line is long we only need to copy data to the string buffer.
            buffer.append(begin, end - begin);
        }

        // Parse data from multiple lines. The fist character will be an
        // OPEN_SQUARE_BRACKET character and last character might be an EOL
        // character.
        void parse(const char *begin, const char *end) {
            ParseScribeTimestamp time_parser;
            const char *ptr = begin;
            const char *start = begin;

			// Find the beginning of a line
			ptr = static_cast<const char *>(memchr(ptr, OPEN_SQUARE_BRACKET, end - ptr));
			if (!ptr) {
				fmt::MemoryWriter writer;
                std::string msg(begin, end - begin);
                writer << "The first character of any line must be [. ";
                writer << std::string(begin, end - begin);
                throw(std::runtime_error(writer.str()));
                return;
			}
			
			// Parse scribe messages
            while (ptr != end) {
                // Find the end of the message header
                ptr = static_cast<const char *>(memchr(ptr, CLOSE_SQUARE_BRACKET, end - ptr));

                if (!ptr) {
                    fmt::MemoryWriter writer;
                    writer << "Could not find the end of the message header. ";
                    writer << std::string(start, end - start);
                    throw(std::runtime_error(writer.str()));
                }

                fmt::print("Header: {}\n", std::string(start, ptr - start - 1));
                std::time_t t = time_parser(start + 1, ptr);
                bool isok = time_constraint(t);

                // Move the the end of line
                ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr));
                if (!ptr) {
                    // We either read the end of a file or have an invalid line.
                    if (isok) { print(start, end); }
                    return;
                }

                // Print out the full message body.
				if (isok) { print(start, ptr); }
                
                // Move to the next line
                start = ++ptr;
            }
        }

        void print(const char *begin, const char *end) {
			if (end == begin) return;
            // fmt::print("{}", std::string(begin, end - begin));
        }

      private:
		size_t lines;
        HeaderConstraint time_constraint;
        MessageConstraint search_pattern;
        std::string buffer;
        static constexpr size_t MAX_SIZE = 1 << 20;
        static constexpr char EOL = '\n';
        static constexpr char SPACE = ' ';
        static constexpr char OPEN_SQUARE_BRACKET = '[';
        static constexpr char CLOSE_SQUARE_BRACKET = ']';
    };
} // namespace scribe
