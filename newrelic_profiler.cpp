/*
+----------------------------------------------------------------------+
| HipHop for PHP                                                       |
+----------------------------------------------------------------------+
| Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
| Copyright (c) 1997-2010 The PHP Group                                |
+----------------------------------------------------------------------+
| This source file is subject to version 3.01 of the PHP license,      |
| that is bundled with this package in the file LICENSE, and is        |
| available through the world-wide-web at the following url:           |
| http://www.php.net/license/3_01.txt                                  |
| If you did not receive a copy of the PHP license and are unable to   |
| obtain it through the world-wide-web, please send a note to          |
| license@php.net so we can mail you a copy immediately.               |
+----------------------------------------------------------------------+
*/

#include "newrelic_profiler.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"

namespace HPHP {
    void NewRelicProfiler::beginFrameEx(const char *symbol) {

        NewRelicProfilerFrame *frame = dynamic_cast<NewRelicProfilerFrame *>(m_stack);

        if (m_stack->m_parent) {
            NewRelicProfilerFrame *p = dynamic_cast<NewRelicProfilerFrame *>(frame->m_parent);
            frame->m_nr_depth = p->m_nr_depth + 1;
        } else {
            frame->m_nr_depth  = 0;
        }
        frame->m_nr_segement_code = 0;
        if (frame->m_nr_depth < max_depth) {
            frame->m_nr_segement_code = newrelic_segment_generic_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, frame->m_name);
        }

    }

    void NewRelicProfiler::endFrameEx(const TypedValue *retval,
    const char *given_symbol) {

        char symbol[512];
        NewRelicProfilerFrame *frame = dynamic_cast<NewRelicProfilerFrame *>(m_stack);
        frame->getStack(2, symbol, sizeof(symbol));

        if (frame->m_nr_segement_code > 0) {
            newrelic_segment_end(NEWRELIC_AUTOSCOPE, frame->m_nr_segement_code);
            frame->m_nr_segement_code = 0;
        }
    }

}
