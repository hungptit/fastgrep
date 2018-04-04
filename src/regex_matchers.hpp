#pragma once

#include "fmt/format.h"
#include "hs/hs.h"
#include <string>

namespace utils {
    namespace hyperscan {
        unsigned int number_of_matches; // TODO: How to avoid of this global variable?
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
                number_of_matches = 0;
                if (hs_scan(database, data.data(), data.size(), 0, scratch, event_handler,
                            ptr) != HS_SUCCESS) {
                    throw std::runtime_error("Unable to scan input buffer");
                }
                return number_of_matches > 0;
            }

            ~RegexMatcher() {
                hs_free_scratch(scratch);
                hs_free_database(database);
            }

          private:
            hs_database_t *database = NULL;
            hs_scratch_t *scratch = NULL;
            std::string pattern;
            static unsigned int counter;
            // An event handle callback.
            static int event_handler(unsigned int id, unsigned long long from,
                                     unsigned long long to, unsigned int flags, void *ctx) {
                fmt::print("Match for pattern {0} at offset {1}\n", (char *)ctx, to);
                ++number_of_matches;
                return 0;
            }
        };
    } // namespace hyperscan
} // namespace utils
