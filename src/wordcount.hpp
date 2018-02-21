#pragma once

#include "scribe.hpp"
#include "unistd.h"

namespace scribe {
    struct FileStats {
        size_t line_count;
        size_t word_count;
        size_t byte_count;
    };

    // A fun class which can be used to count lines, words, and size of a file.
    template <size_t BUFFER_SIZE> class WordCount {
      public:
        FileStats operator()(const char *datafile) {
            size_t byte_count = 0;
            size_t line_count = 0;
            size_t word_count = 0;

            char read_buffer[BUFFER_SIZE + 1];
            int fd = ::open(datafile, O_RDONLY);

            // Check that we can open a given file.
            if (fd < 0) {
                fmt::MemoryWriter writer;
                writer << "Cannot open file \"" << datafile << "\"";
                throw(std::runtime_error(writer.str()));
            }

            // Read data into a string
            while (true) {
                auto nbytes = ::read(fd, read_buffer, BUFFER_SIZE);
                if (nbytes < 0) {
                    fmt::MemoryWriter writer;
                    writer << "Cannot read file \"" << datafile << "\"";
                    throw(std::runtime_error(writer.str()));
                };

                // Update the number of read bytes, lines, and words.
                byte_count += nbytes;
                char *ptr = read_buffer;
                char *end = read_buffer + nbytes;

                // Use memchr to find for a character because we know that our log line is long enough.
                while ((ptr = static_cast<char *>(memchr(ptr, EOL, end - ptr)))) {
                    ++ptr;
                    ++line_count;
                }

                // Stop if we reach the end of file.
                if (nbytes != static_cast<decltype(nbytes)>(BUFFER_SIZE)) { break; };
            }

            // Close our file.
            ::close(fd);

            return {line_count, word_count, byte_count};
        }

      private:
        static constexpr char EOL = '\n';
    };

    // A simple parser which computes the file size, the number of lines, and the maximum/minimum length of
    // lines.
    class LineParser {
      public:
        void operator()(const char *begin, size_t nbytes) {
            file_size += nbytes;
			
            // Count the number of lines
            const char *ptr = begin;
			const char *end = begin + nbytes;
            while (ptr = static_cast<const char *>(memchr(ptr, EOL, end - ptr))) {
                ++ptr;
                ++lines;
            }
        }

        void print(const std::string &title) const {
            fmt::print("File size: {}\n", file_size);
            fmt::print("Number of lines: {}\n", lines);
            fmt::print("Max line length: {}\n", max_len);
            fmt::print("Min line lenght: {}\n", min_len);
            fmt::print("File size: {}\n", file_size);
        }

        size_t file_size = 0;
        size_t lines = 0;
        size_t max_len = 0;
        size_t min_len = 0;
        size_t current_eol = 0;

      private:
        static constexpr char EOL = '\n';
    };

    // A fun class which can be used to count lines, words, and size of a file.
    template <size_t BUFFER_SIZE, typename Parser> class FileReader {
      public:
        Parser operator()(const char *datafile) {
            Parser parser;
            char read_buffer[BUFFER_SIZE + 1];
            int fd = ::open(datafile, O_RDONLY);

            // Check that we can open a given file.
            if (fd < 0) {
                fmt::MemoryWriter writer;
                writer << "Cannot open file \"" << datafile << "\"";
                throw(std::runtime_error(writer.str()));
            }

            // Read data into a string
            while (true) {
                auto nbytes = ::read(fd, read_buffer, BUFFER_SIZE);
                if (nbytes < 0) {
                    fmt::MemoryWriter writer;
                    writer << "Cannot read file \"" << datafile << "\"";
                    throw(std::runtime_error(writer.str()));
                };

                // Parse read_buffer to get some useful information.
                parser(read_buffer, nbytes);

                // Stop if we reach the end of file.
                if (nbytes != static_cast<decltype(nbytes)>(BUFFER_SIZE)) { break; };
            }

            // Close our file.
            ::close(fd);

            return parser;
        }
    };
} // namespace scribe
