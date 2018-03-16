#pragma once

// STD
#include <cstring>
#include <string>

// fmt header
#include "fmt/format.h"

// cereal headers
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/archives/xml.hpp"

// JSON parser
#include "sajson.h"

// Local headers
#include "scribe.hpp"

namespace scribe {

    const char RelayingMessage[] = "Relaying message";
    const char ReceivedMessage[] = "received\n";
    const char StartingExecution[] = "Starting execution.\n";
    const char FinishedInMessage[] = "finished in";

    class MessageBodyParser {
      public:
        bool operator()(char *begin, const size_t len) {
            const sajson::document &doc =
                sajson::parse(sajson::bounded_allocation(ast_buffer, AST_BUFFER_SIZE),
                              sajson::mutable_string_view(len, begin));

            if (doc.is_valid()) {
                fmt::print("Parse success\n");
            } else {
                fmt::print("Parse failed: {}\n", doc.get_error_message_as_cstring());
                return false;
            }

            // Process parse JSON document
            const auto node = doc.get_root();
            assert(node.get_type() == sajson::TYPE_OBJECT);
            auto length = node.get_length();
            fmt::print("length: {}\n", length);
            const sajson::string level_key("LEVEL", 5);
            const sajson::string prefix_key("PREFIX", 6);
            const sajson::string message_key("PREFIX", 6);
            const char *level_value = node.get_value_of_key(level_key).as_cstring();
            const char *msgid = node.get_value_of_key(prefix_key).as_cstring();
			
            // Print out results
            fmt::print("{0} -> {1}\n", level_key.data(), level_value);
            fmt::print("{0} -> {1}\n", prefix_key.data(), msgid);

			if (!strcmp(level_value, "info")) {
				
            } else if (!strcmp(level_value, "error")) {

            } else if (!strcmp(level_value, "warn")) {

            } else {
                // We should never be here.
                assert(0);
            }

            // const sajson::string message("MESSAGE", 7);

            // fmt::print("{0}: {1}\n", message.data(), node.get_value_of_key(message).as_cstring());

            // Return
            return true;
        }

      private:
        static constexpr size_t AST_BUFFER_SIZE = 1 << 12;
        size_t ast_buffer[AST_BUFFER_SIZE];

        // Messages
        std::vector<scribe::ExecutionMessage> execution_messages;
        std::vector<scribe::PublishMessage> control_messages;
        std::vector<scribe::RabbitMQMessage> rabbitmq_messages;
        std::vector<scribe::ErrorMessage> error_messages;
    };
}
