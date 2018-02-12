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

            // Tell the kernel that we will read the current file sequentially.
            // fdadvise (fd, 0, 0, FADVISE_SEQUENTIAL);

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
                while ((ptr = static_cast<char *>(memchr(ptr, '\n', end - ptr)))) {
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
} // namespace scribe
