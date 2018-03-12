#pragma once

#include <array>
#include <string>
#include <vector>

#include "constraints.hpp"
#include "scribe.hpp"

#include "utils/timeutils.hpp"

namespace scribe {
    template <typename Iter> Iter find(Iter begin, Iter end, const char ch) {
        Iter iter(begin);
        for (; (iter != end) && (*iter != ch); ++iter) {}
        return iter;
    }

    // The basic parser will parse and filter messages using basic log
    // information such as timestamp, servername, and process id.
    template <typename TimeConstraint, typename ServerConstraint, typename PoolConstraint>
    class BasicParser {
      public:
        BasicParser(TimeConstraint &&tcons, ServerConstraint &&scons, PoolConstraint &&pcons)
            : time_constraints(tcons), server_constraints(scons), pool_constraints(pcons), data() {}

        // Parse information from a given range.
        template <typename Iter> void operator()(Iter begin, Iter end) {
            std::string timestr;
            utils::TimeParser time_parser("%m/%d/%Y %H:%M:%S");
            utils::TimePrinter printer("%Y-%m-%d %H:%M:%S");
            Iter iter(begin);
            Iter bol;
            while (iter != end) {
                // Find the open square bracket
                iter = find(iter, end, OPEN_SQUARE_BRAKET);
                if (iter == end) return;
                bol = ++iter;

                // Find timestamp string
                iter = find(iter, end, SPACE);
                if (iter == end) return;
                ++iter;
                iter = find(iter, end, SPACE);

                // Compute the time stamp.
                timestr.assign(bol, iter);
                std::time_t timestamp = time_parser(timestr.c_str());

                // Find server name.
                bol = ++iter;
                if (iter == end) return;
                iter = find(iter, end, SPACE);
                std::string server(bol, iter);

                // Find pool name
                bol = ++iter;
                iter = find(iter, end, SPACE);
                std::string pool(bol, iter);

                // Find process id.
                bol = ++iter;
                iter = find(iter, end, CLOSE_SQUARE_BRAKET);
                std::string pid(bol, iter);

                // Update ids
                // auto server_id = data.servers.getidx(server);
                // auto pool_id = data.pools.getidx(pool);

                // Find log message which is in JSON format.
                bol = ++iter;
                iter = find(iter, end, EOL);
                std::string buffer(bol, iter);

                // Display debuging information.
                printer(timestamp);
                fmt::print("[{0} {1} {2}] {3}\n", printer.buffer, server, pid, buffer);

                // Move to the next line.
                if (iter != end) { ++iter; }
            }
        }

      private:
        TimeConstraint time_constraints;
        ServerConstraint server_constraints;
        PoolConstraint pool_constraints;
        ScribeData data;
    };
} // namespace scribe
