#pragma once

namespace fastgrep {
    // Reference: https://misc.flogisoft.com/bash/tip_colors_and_formatting
    static const std::string BOLD_GREEN = "\033[1;32m"; // Normal, Green
    static const std::string BOLD_BLUE = "\033[1;34m";  // Bold, Blue
    static const std::string BOLD_WHITE = "\033[1;97m"; // Normal, white
    static const std::string RESET_COLOR = "\033[0m";   // Reset
    constexpr char EOL = '\n';
} // namespace fastgrep
