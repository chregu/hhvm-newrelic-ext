#ifndef incl_HPHP_NEWRELIC_PROFILER_H_
#define incl_HPHP_NEWRELIC_PROFILER_H_


#include "hphp/runtime/ext/ext_hotprofiler.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/server/server-stats.h"
#include "newrelic_transaction.h"
#include "newrelic_collector_client.h"
#include "newrelic_common.h"

namespace HPHP {
    ///////////////////////////////////////////////////////////////////////////////


    class NewRelicProfiler : public Profiler {

        public:
        explicit NewRelicProfiler(int flags) : m_flags(flags) {
                max_depth = flags;
        }


        virtual void beginFrameEx(const char *symbol);
        virtual void endFrameEx(const TypedValue *retval,
                          const char *given_symbol);
        virtual void endAllFrames();
        private:

        typedef hphp_hash_map<std::string, string_hash> StatsMap;
          StatsMap m_stats; // outcome

        int max_depth;
        uint32_t m_flags;
    };



}

#endif // incl_HPHP_NEWRELIC_PROFILER_H_