#pragma once
#include <array>
#include <ctime>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// Time parser and printer
#include "timeutils.hpp"

namespace scribe {

    // This class hold a look up table for string.
    // Note: This class is not threadsafe.
    template <typename T1, typename T2 = size_t> struct LookupTable {
        using key_type = T1;
        using index_type = T2;
        using map_type = std::unordered_map<key_type, index_type>;

        template <typename V> index_type getidx(V &&key) {
            auto const it = lookup_table.find(key);
            if (it == lookup_table.end()) {
                const index_type idx = static_cast<index_type>(data.size());
                lookup_table.emplace(key, idx);
                data.emplace_back(key);
                return idx;
            } else {
                return it->second;
            }
        }

        std::vector<key_type> data;
        map_type lookup_table;
    };

    enum class MessageType : uint8_t { INFO = 0, WARNING = 1, ERROR = 2, RAW_ERROR = 3 };

    // Enum values that represent the life cycle of messages. This can be used
    // to keep track of messages.
    enum class MessageLifeCycle : int8_t {
        PUBLISH = 0,
        RELAYING = 1,
        DROPPED = 2,
        REPUBLISHING = 3,
        RECEIVED = 4,
        START_EXECUTING = 5,
        FINISHED = 6,
    };

    // TODO: We can pack message life cycle into a single integer number for
    // example {PUBLISH, RELAYING, RECEIVED, START_EXECUTING, FINSIHED} will be converted to
    // [x,x,x,x,x,x,x,x] where [PUBLISH] is the number of published requests etc. We only need 8 bytes for
    // the whole sequence. We can define a map to hold message life cycle msgid -> status.
    // TODO: The message life cycle map can be saved into SQLite for example for have 24 tables with below
    // columns messageid, timestamp, cycle, pool, server.

    using PoolDB = LookupTable<std::string, unsigned int>;
    using ServerDB = LookupTable<std::string, unsigned int>;
    using MessageDB = LookupTable<std::string, size_t>;

    // This data structure has basic information about a scribe log message.
    struct ScribeHeader {
        std::time_t timestamp;
        unsigned int pool_id;
        unsigned int server_id;
    };

    // The body of the Scribe log message.
    struct ScribeBody {
        MessageType typpe;
        size_t msgid;
        std::string data;
    };

    // Will be used to analyze message life cycle.
    struct MessageStatus {
        std::array<unsigned char, 8> life_cycles;
        std::array<char, 22> message_id; // MessageID is a 22 character string.
    };

    struct ScribeMessage {
        ScribeHeader header;
        ScribeBody message;
    };

    struct ScribeData {
        std::vector<ScribeHeader> headers;
        std::vector<ScribeMessage> messages;
        PoolDB pools;
        ServerDB servers;
        MessageDB msgids;
    };

    template <typename Iter> Iter find(Iter begin, Iter end, const char ch) {
        Iter iter(begin);
        for (; (iter != end) && (*iter != ch); ++iter) {}
        return iter;
    }
    constexpr char EOL = '\n';
    constexpr char SPACE = ' ';
    constexpr char OPEN_SQUARE_BRAKET = '[';
    constexpr char CLOSE_SQUARE_BRAKET = ']';

    // The basic parser will parse and filter messages using basic log
    // information such as timestamp, servername, and process id.
    class BasicParser {
      public:
        // Parse information from a given range.
        template <typename Iter> void operator()(Iter begin, Iter end) {
            std::string timestr;
            utils::TimeParser time_parser("%m/%d/%Y %H:%M:%S");
            utils::TimePrinter printer("%Y-%m-%d %H:%M:%S");
            Iter iter(begin);
            while (iter != end) {
                Iter bol(iter);

                // First character of the current line is '['
                if (*iter != OPEN_SQUARE_BRAKET) return;
                ++iter; // Move to the next character

                iter = find(iter, end, SPACE);
                if (iter == end) return;
                ++iter;
                iter = find(iter, end, SPACE);

                // Compute the time stamp.
                timestr.assign(begin, iter);
                std::time_t timestamp = time_parser(timestr.c_str());

                // Find server name.
                bol = ++iter;
                if (iter == end) return;

                iter = find(iter, end, SPACE);
                std::string server(bol, iter);
                auto server_id = data.servers.getidx(server);

                // Find pool name
                bol = ++iter;
                iter = find(iter, end, SPACE);
                std::string pool(bol, iter);

                // Find process id.
                bol = ++iter;
                iter = find(iter, end, CLOSE_SQUARE_BRAKET);
                std::string pid(bol, iter);

                // Find log message which is in JSON format.
                bol = ++iter;
                iter = find(iter, end, EOL);
                std::string buffer(bol, end);

                // Display debuging information.
                printer(timestamp);
                fmt::print("[{0} {1} {2}] {3}\n", timestr, server, pid, buffer);

                // Move to the next line.
                if (iter != end) { ++iter; }
            }
        }

      private:
        ScribeData data;
    };
}
