#pragma once

#include "scribe.hpp"

namespace scribe {
	template <size_t BUFFER_SIZE>
    class ScribeParser {
      public:
        size_t operator()(const std::string &datafile) {
            size_t byte_count = 0, line_count = 0;

            int fd = ::open(datafile.c_str(), O_RDONLY);

            // Check that we can open a given file.
            if (fd < 0) {
                fmt::MemoryWriter writer;
                writer << "Cannot open file \"" << datafile << "\"";
                throw(std::runtime_error(writer.str()));
            }

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
				int idx;
                for (idx = 0; idx < nbytes; ++idx) {
                    // Increate line_count if we see any EOL chacter. We also
                    // need to update the current word count.
                    if (read_buffer[idx] == EOL) {
                        ++line_count;
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

			return byte_count;
        }

    private:
        // Constants
        static constexpr char EOL = '\n';
        static constexpr char SPACE = ' ';
        // static constexpr size_t BUFFER_SIZE = 1 << 16;

        // Temporary read buffer.
        std::array<char, BUFFER_SIZE> read_buffer;
    };
} // namespace scribe
