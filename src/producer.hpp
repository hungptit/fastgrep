#pragma once

#include "scribe.hpp"
#include "unistd.h"

#include <algorithm>
#include <deque>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

#include "scribe.hpp"

#include "timeutils.hpp"

namespace scribe {
	struct SimpleScribeMessage {
        std::time_t timestamp;
        std::string node_name;
        std::string info;
        std::string message;
        size_t pid;
    };

    class SimpleScribeMessagePrinter {
      public:
        SimpleScribeMessagePrinter(const std::string &fmt) : msg_counter(10), time_printer(fmt) {}
		SimpleScribeMessagePrinter(SimpleScribeMessagePrinter &) = delete;
		
        void print(const SimpleScribeMessage &msg) {
            time_printer(msg.timestamp);
			++msg_counter;
            writer << "[" << time_printer.buffer << " " << msg.node_name << " " << msg.info << "  "
                   << msg.pid << "]" << msg.message << "\n";
            if (msg_counter > msg_num) {
                fmt::print("{}", writer.str());
                writer.clear();
                msg_counter = 0;
            }
            fmt::print("{0} : {1}, ", msg_counter, msg_num);
        }

      private:
        static constexpr size_t msg_num = 10000;
        size_t msg_counter;
        utils::TimePrinter time_printer;
        fmt::MemoryWriter writer;
    };

    // A fun class which can be used to count lines, words, and size of a file.
    template <size_t READ_BUFFER_SIZE> class MessageFilter {
      public:
        // This method has one producer and multiple consumers model.
        size_t operator()(const char *datafile) { return read(datafile); }

        int parse_number(const char *ptr, const char *end) {
            int num = 0;
            while (true) {
                if (ptr == end) return num;
                char ch = *ptr;
                if (!std::isalnum(ch)) { return num; }
                num = num * 10 + ch - '0';
                ++ptr;
            }
        }

        void parse(std::string &&data) {
			fmt::print("{}", data);
            char *ptr = &data[0];
            char *end = ptr + data.size(); // End of the data bufer.
            struct tm tm;
			char timestamp[20];
            SimpleScribeMessagePrinter printer("%Y-%m-%d %H:%M:%S");
            while (ptr != end) {
				// Assume this is the begining of a line
				const char *line_begin = ptr;
				char *begin = ++ptr; // Skip [
				assert(ptr);
				
				char *ptr = static_cast<char *>(memchr(ptr, ' ', end - ptr));
				fmt::print("Timestamp: {}", data.size());
				fmt::print("Timestamp: {}", end - begin);
				assert(ptr);
				
				ptr = static_cast<char *>(memchr(ptr, ' ', end - ptr));
				assert(ptr);



				// memcpy(timestamp, begin, ptr - begin);
				// return;
				
				// if (!ptr) return;
				
                // SimpleScribeMessage msg;

                // // Get the time stamp.
				// fmt::print("Parse time\n");
				// ptr = static_cast<char *>(memchr(ptr, ' ', end - ptr));
                // ptr = strptime(++ptr, "%m/%d/%Y %H:%M:%S", &tm);
                // if (!ptr) return; // Return early if we could not parse the timestamp.
                // msg.timestamp = mktime(&tm);
				
                // begin = ++ptr;

                // // Parse the node name
				// fmt::print("Parse sever name\n");
                // ptr = static_cast<char *>(memchr(ptr, ' ', end - ptr));
                // if (!ptr) return;
                // msg.node_name.assign(begin, ptr - begin);
                // begin = ++ptr;

                // // Parse information which might be a pool name or message type etc.
				// fmt::print("Parse item info\n");
                // ptr = static_cast<char *>(memchr(ptr, ' ', end - ptr));
                // if (!ptr) return;
                // msg.info.assign(begin, ptr - begin);
                // begin = ++ptr;

                // // Parse the process id.
				// fmt::print("Parse item process id\n");
                // ptr = static_cast<char *>(memchr(ptr, ']', end - ptr));
                // msg.pid = parse_number(begin, ptr);
                // if (!ptr) return;
                // ++ptr;

                // // Get the message
				// fmt::print("message -> {}\n", ptr);
                // begin = ++ptr;
                // if (!ptr) return;
                // ptr = static_cast<char *>(memchr(ptr, EOL, end - ptr));
                // if (!ptr) return;
                // msg.message.assign(begin, ptr - begin);

                // // Print out parsed results
                // printer.print(msg);

                // Move to the next line.
                ++ptr;
				fmt::print("message -> {}\n", ptr);
				break;
            }
        }

        // This algorithm will
        // 1. Read data from a file into read buffer.
        // 2. Copy data from a read buffer into a write buffer line by line.
        // 3. Copy data to a consumer queues if the number of lines is exceed NLINES.
        // 4. Other notes
        //    + The size of a line can be big (>1Mbytes).
        //    + Parsed files are very big > 5GBytes.
        //    + The expected parsing rate should be > 100MBytes/sec.
        size_t read(const char *datafile) {
            size_t byte_count = 0, line_count = 0;
            char read_buffer[READ_BUFFER_SIZE + 1];
            int fd = ::open(datafile, O_RDONLY);

            // Check that we can open a given file.
            if (fd < 0) {
                fmt::MemoryWriter writer;
                writer << "Cannot open file \"" << datafile << "\"";
                throw(std::runtime_error(writer.str()));
            }

            size_t lines = 0;
            std::string write_buffer;
            while (true) {
                // Read READ_BUFFER_SIZE bytes from an input file.
                auto nbytes = ::read(fd, read_buffer, READ_BUFFER_SIZE);
                if (nbytes < 0) {
                    fmt::MemoryWriter writer;
                    writer << "Cannot read file \"" << datafile << "\"";
                    throw(std::runtime_error(writer.str()));
                };

                // Update the number of read bytes.
                byte_count += nbytes;

                // Count the number of lines found and assign iter to the last found eol.
                char *ptr = read_buffer;
                char *iter = read_buffer;
                const char *end = read_buffer + nbytes;
                while ((ptr = static_cast<char *>(memchr(ptr, EOL, end - ptr)))) {
                    ++ptr;
                    ++lines;
                    iter = ptr;
                }

                // We have not reach the line limit yet. Copy the whole buffer
                // to the write buffer. Otherwise copy all valid lines to the
                // write buffer, save the write buffer, then copy leftover data
                // to the fresh write buffer.
                if (lines < LINES) {
                    write_buffer.append(read_buffer, nbytes);
                } else {
                    line_count += lines;
                    // Copy lines to a string buffer.
                    size_t copied_bytes = iter - read_buffer;
                    write_buffer.append(read_buffer, copied_bytes);

                    // Push lines into a consumer queue.
                    {
                        std::lock_guard<std::mutex> guard(queue_lock);
                        parse(std::move(write_buffer));
                    }

                    // Copy leftover bytes to the string buffer.
                    write_buffer.clear();
                    if (static_cast<size_t>(nbytes) > copied_bytes) {
                        write_buffer.append(iter, nbytes - copied_bytes);
                    }

                    // Reset the line counter;
                    lines = 0;
                }

                // Stop if we reach the end of file.
                if (nbytes != static_cast<decltype(nbytes)>(READ_BUFFER_SIZE)) {
                    message_queue.emplace_back(std::move(write_buffer));
                    break;
                };
            }

            // Close our file.
            ::close(fd);

            // Return the file size and the number of line.
            return byte_count;
        }

      private:
        std::mutex queue_lock;
        std::deque<std::string> message_queue;
        std::vector<std::string> messages;
        static constexpr char EOL = '\n';
        static constexpr char SPACE = ' ';
        static constexpr char OPEN_SQUARE_BRACKET = '[';
        static constexpr char CLOSE_SQUARE_BRACKET = ']';
        static constexpr size_t LINES = 100;
    };
} // namespace scribe
