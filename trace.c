#include "trace.h"

char trace_out[1024 * 1024];
trace_ctx_t trace_ctx_val = (trace_ctx_t){
    .out = &trace_out[0],
    .cursor = 0,
    .active_span = 0,
    .span_ids = {0},
};
trace_ctx_t *trace_ctx = &trace_ctx_val;

trace_ctx_t *get_trace_ctx() { return trace_ctx; }
