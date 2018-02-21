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
	// Constants
	constexpr char EOL = '\n';
    constexpr char SPACE = ' ';
	constexpr char COMMA = ',';
	constexpr char COMMA1 = ';';
	constexpr char PERIOD = '.';
    constexpr char OPEN_SQUARE_BRAKET = '[';
    constexpr char CLOSE_SQUARE_BRAKET = ']';
	
	
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
		size_t pid;
    };

    // The body of the Scribe log message.
    struct ScribeBody {
        MessageType type;
        size_t msgid;
        std::string data;
    };

    // Will be used to analyze message life cycle.
    struct MessageStatus {
        std::array<int8_t, 8> life_cycles;
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
}
