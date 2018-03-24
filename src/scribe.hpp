#pragma once
#include <array>
#include <ctime>
#include <string>
#include <vector>

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/archives/xml.hpp"

namespace scribe {
    // A scribe header has this format "[03/08/2018 12:00:00 node1234.example.com generic.workqueue 123456]"
    struct MessageHeader {
        std::time_t timestamp; // Timestamp: 03/08/2018 23::00:50
        std::string server;    // A server address: job1120.domain_name.com
        std::string pool;      // A job pool name i.e job.pool.name
        long pid;              // This is a process id

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(timestamp), CEREAL_NVP(server), CEREAL_NVP(pool), CEREAL_NVP(pid));
        }
    };

    // Control message types
    enum class ControlMessageType : int8_t {
        PUBLISH = 0,
        RELAYING = 1,
        DROPPED = 2,
        REPUBLISHING = 3,
        RECEIVED = 4,
        START_EXECUTING = 5,
        FINISHED = 6,
    };

    enum class ErrorMessageType : int8_t {
        CONNECTION = 0,
        EXCEPTION = 1,
    };

    enum class RabbitMQMessageType : int8_t {
        PUBLISH = 0,
    };

    using MessageID = std::string;

    // Control message states. This can help us to track the life cycle of any message.
    struct ExecutionMessage {
        MessageID msgid;
        std::array<int8_t, 8> states; // Store states of a messages i.e publish, relaying, received etc.
        template <typename Archive> void serialize(Archive &ar) { ar(CEREAL_NVP(msgid), CEREAL_NVP(states)); }
    };

    struct PublishMessage {
        MessageID msgid;
        std::string resource;
        std::string request;

        template <typename Archive> void serialize(Archive &ar) { ar(CEREAL_NVP(msgid)); }
    };

    // Publish message
    struct RabbitMQMessage {
        RabbitMQMessageType type;
        MessageID msgid;
        std::string message;

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(type), CEREAL_NVP(msgid), CEREAL_NVP(message));
        }
    };

    struct ErrorMessage {
        MessageID msgid;
        std::string message;

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(msgid), CEREAL_NVP(message));
        }
    };

	struct RawErrorMessage {
		MessageID msgid;
		std::string message;
		std::string subject;
		std::string cluster;

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(msgid), CEREAL_NVP(message), CEREAL_NVP(subject), CEREAL_NVP(cluster));
        }
	};
	
} // namespace scribe
