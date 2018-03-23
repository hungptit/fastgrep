#pragma once

#include <cstring>
#include <ctime>
#include <string>

namespace scribe {
    // Time constraint
    struct TimeAll {
        bool operator()(const std::time_t) { return true; }
    };

    struct OlderThan {
        bool operator()(const std::time_t t) { return t <= timestamp; }
        std::time_t timestamp;
    };

    struct NewerThan {
        bool operator()(const std::time_t t) { return t >= timestamp; }
        std::time_t timestamp;
    };

    struct Between {
        bool operator()(const std::time_t t) { return (t >= begin) && (t <= end); }
        std::time_t begin, end;
    };



} // namespace scribe
