#include "fmt/format.h"
#include <string>

#include "constants.hpp"
#include "ioutils/ioutils.hpp"
#include "output.hpp"

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

TEST_CASE("FMTPolicy") {
    fastgrep::FMTPolicy console;
    std::string data("Hello world");
    const unsigned int linenum = 7;
    console.print_color_text(data.data(), data.data() + data.size(), linenum);
    console.print_plain_text(data.data(), data.data() + data.size(), linenum);
    console.print_color_text(data.data(), data.data() + data.size());
    console.print_plain_text(data.data(), data.data() + data.size());
}

TEST_CASE("DirectPolicy") {
    fastgrep::DirectPolicy console;
    std::string data("Hello world");
    const unsigned int linenum = 7;
    console.print_color_text(data.data(), data.data() + data.size(), linenum);
    console.print_plain_text(data.data(), data.data() + data.size(), linenum);
    console.print_color_text(data.data(), data.data() + data.size());
    console.print_plain_text(data.data(), data.data() + data.size());
}

TEST_CASE("StorePolicy") {
    fastgrep::StorePolicy console;
    std::string data("Hello world");
    const unsigned int linenum = 7;
    console.print_color_text(data.data(), data.data() + data.size(), linenum);
    REQUIRE(console.linenums.size() == 1);
    REQUIRE(console.linenums.size() == 1);
    
    console.print_plain_text(data.data(), data.data() + data.size(), linenum);
    REQUIRE(console.lines.size() == 2);
    REQUIRE(console.linenums.size() == 2);
    
    console.print_color_text(data.data(), data.data() + data.size());
    REQUIRE(console.lines.size() == 3);
    
    console.print_plain_text(data.data(), data.data() + data.size());
    REQUIRE(console.lines.size() == 4);
}
