#pragma once

#include <string>
#include "fmt/format.h"
#include "hs/hs.h"

namespace utils {
    class RegexMatcher {
      public:
        RegexMatcher(const std::string &patt) {
            pattern = patt;
            hs_compile_error_t *compile_err;
            if (hs_compile(pattern.c_str(), HS_FLAG_DOTALL, HS_MODE_BLOCK, NULL, &database,
                           &compile_err) != HS_SUCCESS) {
                fmt::MemoryWriter writer;
                writer << "ERROR: Unable to compile pattern \"" << pattern
                       << "\": " << compile_err->message;
                throw std::runtime_error(writer.str());
            }

            if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
                fmt::MemoryWriter writer;
                throw std::runtime_error("Unable to allocate scratch space.");
            }
        }

        bool operator()(const std::string &data) {
            char *ptr = const_cast<char *>(pattern.c_str());
            return hs_scan(database, data.data(), data.size(), 0, scratch,
                           RegexMatcher::event_handler, ptr) == HS_SUCCESS;
        }

        ~RegexMatcher() {
            hs_free_scratch(scratch);
            hs_free_database(database);
        }

      private:
        hs_database_t *database = NULL;
        hs_scratch_t *scratch = NULL;
        std::string pattern;

        // An event handle callback.
        static int event_handler(unsigned int id, unsigned long long from,
                                 unsigned long long to, unsigned int flags, void *ctx) {
			// fmt::print("Match for pattern {0} at offset {1}", (char *)ctx, to);
            return 0;
        }
    };
}
