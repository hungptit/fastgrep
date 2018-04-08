#pragma once

#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fmt/format.h"

namespace scribe {
    // A struct that read file content in fixed size chunks and parse them to a parser.
    template <size_t BUFFER_SIZE, typename Parser> class FileReader {
      public:
        void operator()(const char *datafile, Parser &parser, const long offset = 0) {
            char read_buffer[BUFFER_SIZE + 1];
            int fd = ::open(datafile, O_RDONLY);

            // Check that we can open a given file.
            if (fd < 0) {
                std::stringstream writer;
                writer << "Cannot open file \"" << datafile << "\"";
                throw(std::runtime_error(writer.str()));
            }

            // Shift to desired location if it is not zero.
            if (offset) {
                auto retval = lseek(fd, offset, SEEK_SET);
                if (retval != offset) {
                    std::stringstream writer;
                    writer << "Cannot seek for the location " << offset << " in " << datafile;
                    throw(std::runtime_error(writer.str()));
                }
            }

			// Let the kernel know that we are going to read sequentially to the end of a file.
			// posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
			
            // Read data into a string
            while (true) {
                auto nbytes = ::read(fd, read_buffer, BUFFER_SIZE);
                if (nbytes < 0) {
                    std::stringstream writer;
                    writer << "Cannot read file \"" << datafile << "\"";
                    throw(std::runtime_error(writer.str()));
                };

                // Parse read_buffer to get some useful information.
                parser(read_buffer, read_buffer + nbytes);

                // Stop if we reach the end of file.
                if (nbytes != static_cast<decltype(nbytes)>(BUFFER_SIZE)) { break; };
            }

            // Close our file.
            ::close(fd);
        }
    };
} // namespace scribe
