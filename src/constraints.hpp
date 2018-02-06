#pragma once

#include <string>

namespace scribe {
    struct All {
        bool operator()() { return true; }
    };

    struct OlderThan {
        bool operator()(const std::time_t begin) { return begin <= timestamp; }
		std::time_t timestamp;
    };

    struct NewerThan {
        bool operator()(const std::time_t end) { return end >= timestamp; }
		std::time_t timestamp;
    };
}
