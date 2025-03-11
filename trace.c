#include "trace.h"

bool tracing_enabled = false;

char trace_out[TRACE_MAX_OUT];
trace_ctx_t trace_ctx_val = (trace_ctx_t){
    .out = &trace_out[0],
    .span_cursor = 0,
    .active_span = 0,
    .span_ids = {0},
    .span_starts = {0},
};
trace_ctx_t *trace_ctx = &trace_ctx_val;

trace_ctx_t *get_trace_ctx() { return trace_ctx; }

uint64_t timespec_ms(struct timespec ts) {
  return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

void enable_tracing() { tracing_enabled = true; }
void disable_tracing() { tracing_enabled = false; }
