#pragma once

#include "scribe.hpp"

namespace scribe {
    struct FileStats {
        size_t line_count;
        size_t word_count;
        size_t byte_count;
    };

	// A fun class which can be used to count lines, words, and size of a file.
	template <size_t BUFFER_SIZE>
    class WordCount {
      public:
        FileStats operator()(const char * datafile) {
            FileStats stats;
            size_t byte_count = 0;
            size_t line_count = 0;
            size_t word_count = 0;

            int fd = ::open(datafile, O_RDONLY);

            // Check that we can open a given file.
            if (fd < 0) {
                fmt::MemoryWriter writer;
                writer << "Cannot open file \"" << datafile << "\"";
                throw(std::runtime_error(writer.str()));
            }

            // Reserve the size of a buffer using file size information.
            struct stat file_stat;
            if (fstat(fd, &file_stat) < 0) return stats;

            // Read data into a string
            int start = 0;
            while (true) {
                auto nbytes = ::read(fd, &read_buffer[0], BUFFER_SIZE);
                if (nbytes < 0) {
                    fmt::MemoryWriter writer;
                    writer << "Cannot read file \"" << datafile << "\"";
                    throw(std::runtime_error(writer.str()));
                };

                // Update the number of read bytes, lines, and words.
                byte_count += nbytes;
                for (auto idx = 0; idx < nbytes; ++idx) {
                    // Increate line_count if we see any EOL chacter. We also
                    // need to update the current word count.
                    if (read_buffer[idx] == EOL) {
                        ++line_count;
                        (idx > start) ? (++word_count, start = idx) : (start = idx);
                    }

                    // Update word_count;
                    if (read_buffer[idx] == SPACE) {
                        (idx > start) ? (++word_count, start = idx) : (start = idx);
                    }
                }

                // If there are character left then we set start to be negative number otherwise set it to
                // 0.
                start -= nbytes - 1;

                // Stop if we reach the end of file.
                if (nbytes != static_cast<decltype(nbytes)>(BUFFER_SIZE)) { break; };
            }

            // Close our file.
            ::close(fd);

            return {line_count, word_count, byte_count};
        }

    private:
        // Constants
        static constexpr char EOL = '\n';

        // Temporary read buffer.
        std::array<char, BUFFER_SIZE> read_buffer;
    };
} // namespace scribe
