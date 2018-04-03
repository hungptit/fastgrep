#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "scribe.hpp"

#include "utils/timestamp.hpp"

namespace scribe {
    // Parser the scribe message header.
    class MessageHeaderParser {
      public:
        MessageHeader operator()(const char *begin, const char *end) {
            assert((begin + 32) < end);
            const char *ptr = begin + 1;
            MessageHeader header;
            header.timestamp = utils::parse_timestamp<utils::Timestamp>(ptr);
            ptr += 20;

            // Parse server name
            const char *pos = static_cast<const char *>(memchr(ptr, SPACE, end - ptr));
            assert(pos != NULL);
            header.server.append(ptr, pos - ptr);

            // Parse pool name.
            ptr = pos + 1;
            assert(ptr < end);
            pos = static_cast<const char *>(memchr(ptr, SPACE, end - ptr));
            assert(pos != NULL);
            header.pool.append(ptr, pos - ptr);

            // Parse process id.
            ptr = pos + 1;
            assert(ptr < end);
            header.pid = atoi(ptr);

            return header;
        }

      private:
        static constexpr char SPACE = ' ';
    };
} // namespace scribe
