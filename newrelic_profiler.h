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

    const StaticString
    s__SERVER("_SERVER"),
    s__REQUEST_URI("REQUEST_URI"),
    s__SCRIPT_NAME("SCRIPT_NAME"),
    s__NEWRELIC("newrelic");

    class NewRelicProfiler : public Profiler {

        public:
        explicit NewRelicProfiler(int64_t mdepth) :  Profiler(true), max_depth(mdepth)  {
        }


        virtual void beginFrameEx(const char *symbol);
        virtual void endFrameEx(const TypedValue *retval, const char *given_symbol);
        private:


        int64_t max_depth;

    };




}

#endif // incl_HPHP_NEWRELIC_PROFILER_H_