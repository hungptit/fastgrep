#pragma once

#include "cereal/archives/json.hpp"
#include <string>

namespace fastgrep {
    struct Params {
        bool linenum = true;        // Display line number.
        bool inverse_match = false; // Inverse match i.e display lines that do not match given pattern.
        bool exact_match = false;   // Use exact matching algorithm.
        bool ignore_case = false;   // Ignore case.
        bool use_memmap = true;     // Read the file content using memory mapped approach.
        bool utf8 = false;          // Support UTF8.
        bool color = false;         // Display color text.
        bool stream = false;        // grep the input stream.
        bool verbose = false;       // Display verbose information.

        template <typename Archive> void serialize(Archive &ar) {
            ar(CEREAL_NVP(linenum), CEREAL_NVP(inverse_match), CEREAL_NVP(exact_match),
               CEREAL_NVP(ignore_case), CEREAL_NVP(use_memmap), CEREAL_NVP(utf8), CEREAL_NVP(color),
               CEREAL_NVP(stream), CEREAL_NVP(verbose));
        }
    };
} // namespace fastgrep
