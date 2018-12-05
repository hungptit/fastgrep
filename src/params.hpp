#pragma once
#include "fmt/format.h"

namespace fastgrep {
    enum PARAMS : int32_t {
        VERBOSE = 1,
        COLOR = 1 << 1,
        EXACT_MATCH = 1 << 2,
        USE_MEMMAP = 1 << 3,
        UTF8 = 1 << 4,
        UTF16 = 1 << 5,
        UTF32 = 1 << 6,
        LINENUM = 1 << 7
    };

    struct Params {
        int info;
        int regex_mode;
        bool verbose() const { return (info & VERBOSE) > 0; }
        bool color() const { return (info & COLOR) > 0; }
        bool use_memmap() const { return (info & USE_MEMMAP) > 0; }
        bool exact_match() const { return (info & EXACT_MATCH) > 0; }
        bool linenum() const { return (info & LINENUM) > 0; }
        bool utf8() const { return (info & UTF8) > 0; }
        bool utf16() const { return (info & UTF16) > 0; }
        bool utf32() const { return (info & UTF32) > 0; }
        void print() const {
            fmt::print("verbose: {}\n", verbose());
            fmt::print("color: {}\n", color());
            fmt::print("use_memmap: {}\n", use_memmap());
            fmt::print("linenum: {}\n", linenum());
            fmt::print("exact_match: {}\n", exact_match());
            fmt::print("regex_mode: {}\n", regex_mode);
            fmt::print("utf8: {}\n", utf8());
            fmt::print("utf16: {}\n", utf16());
            fmt::print("utf32: {}\n", utf32());
        }
    };
} // namespace fastgrep
