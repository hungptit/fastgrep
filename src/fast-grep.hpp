#pragma once

#include "ioutils/fdwriter.hpp"
#include "ioutils/filesystem.hpp"
#include "utils/regex_matchers.hpp"

namespace fastgrep {
    namespace experiments {
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
                grep(p.data()); // TODO: Need to speed up this call.
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
