#pragma once

#include "ioutils/fdwriter.hpp"
#include "ioutils/filesystem.hpp"
#include "utils/regex_matchers.hpp"

namespace fastgrep {
    namespace experiments {
        // Reference: https://misc.flogisoft.com/bash/tip_colors_and_formatting
        static const std::string FIFO_COLOR = "\033[2;32m";    // Normal, Green
        static const std::string DIR_COLOR = "\033[1;91m";     // Bold, Light red
        static const std::string CHR_COLOR = "\033[2;33m";     // Normal, Yellow
        static const std::string SYMLINK_COLOR = "\033[2;34m"; // Blink, Blue
        static const std::string BLK_COLOR = "\033[1;35m";     // Bold, Magenta
        static const std::string SOCK_COLOR = "\033[2;36m";    // Normal, Cyan
        static const std::string WHT_COLOR = "\033[2;37m";     // Normal, Light gray
        static const std::string FILE_COLOR = "\033[2;97m";    // Normal, white
        static const std::string RESET_COLOR = "\033[0m";      // Reset

        /**
         * Note: This class will be served as the policy for the ioutils::filesystem::DefaultSearch.
         *
         */
        template <typename TextSearchPolicy> struct GrepSearchPolicy {
          public:
            template <typename Params>
            GrepSearchPolicy(Params &&params)
                : color(params.color()), donot_ignore_git(params.donot_ignore_git()), number_of_files(0),
                  path_matcher(params.path_regex, HS_FLAG_DOTALL | HS_FLAG_SINGLEMATCH), grep(params.regex, params) {}

          protected:
            using PathMatcher = utils::hyperscan::RegexMatcher;
            using Path = ioutils::Path;
            bool is_valid_dir(const char *dname) const { return donot_ignore_git ? 1 : (strcmp(dname, ".git") != 0); }

            void process_file(const Path &parent, const char *stem) {
                const std::string p = parent.path + "/" + stem;
                process_file(p);
            }

            void process_file(const Path &parent) { process_file(parent.path); }

            void process_file(const std::string &p) {
                if (!path_matcher.is_matched(p.data(), p.size())) return;
                grep(p.data());
                ++number_of_files;
            }

            // Symlink logic will be handled by the file travesal algorithm.
            void process_symlink(const Path &, const char *) {}
            void process_symlink(const Path &) {}

            // We are not interrested in directory
            void process_dir(const std::string &) {}

            // We are not interrested in FIFO
            void process_fifo(const Path &, const char *) {}
            void process_fifo(const Path &) {}

            // We are not interrested in character special
            void process_chr(const Path &, const char *) {}
            void process_chr(const Path &) {}

            // We are not interrested in process block special
            void process_blk(const Path &, const char *) {}
            void process_blk(const Path &) {}

            // We are not interrested in sockets
            void process_socket(const Path &, const char *) {}
            void process_socket(const Path &) {}

            // We are not interrested in whiteout files
            void process_whiteout(const Path &, const char *) {}
            void process_whiteout(const Path &) {}

            // We do not know what to do with the unknown types in general.
            void process_unknown(const Path &, const char *) {}
            void process_unknown(const Path &) {}

          private:
            bool color;
            bool donot_ignore_git;
            int number_of_files = 0;
            PathMatcher path_matcher;
            TextSearchPolicy grep;
        };
    } // namespace experiments
} // namespace fastgrep
