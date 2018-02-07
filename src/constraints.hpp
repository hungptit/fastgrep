#pragma once

#include <string>
#include <ctime>

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

	// Server constraints
	struct AllServers {
		template <typename Iter>
		bool operator()(Iter begin, Iter end) { return true; }
	};

	struct IncludeServers {
		template <typename Iter>
		bool operator()(Iter begin, Iter end) { return true; }
		std::string pattern;
	};

	struct ExcludeServers {
		template <typename Iter>
		bool operator()(Iter begin, Iter end) { return true; }
		std::string pattern;
	};

	struct AllPools {
		template <typename Iter>
		bool operator()(Iter begin, Iter end) { return true; }
	};
}
