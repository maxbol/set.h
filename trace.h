#ifndef TRACE_H
#define TRACE_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

#define TRACE_MAX_DEPTH 10
#define TRACE_MAX_ITEMS 100
#define TRACE_MAX_STRLEN 250
#define TRACE_MAX_OUT 1024 * 1024

typedef struct {
  uint64_t time;
  uint64_t count;
} trace_latency_hist_t;

typedef struct {
  uint32_t active_span;
  uint32_t exceed_len;
  char *out;
  size_t span_cursor;
  uint64_t span_ids[TRACE_MAX_DEPTH];
  uint64_t trace_ids[TRACE_MAX_DEPTH];
  uint64_t span_starts[TRACE_MAX_DEPTH];
  trace_latency_hist_t out_hist[TRACE_MAX_ITEMS];
} trace_ctx_t;

extern trace_ctx_t *get_trace_ctx();
extern void enable_tracing();
extern void disable_tracing();
extern bool tracing_enabled;
extern uint64_t timespec_ms(struct timespec ts);

#define trace_printf(...)                                                      \
  do {                                                                         \
    trace_ctx_t *ctx = get_trace_ctx();                                        \
    if (ctx->span_cursor >= TRACE_MAX_OUT) {                                   \
      break;                                                                   \
    }                                                                          \
    ctx->span_cursor +=                                                        \
        snprintf(&ctx->out[ctx->span_cursor],                                  \
                 TRACE_MAX_OUT - ctx->span_cursor, __VA_ARGS__);               \
  } while (0)

#define trace_indent(indent, ...)                                              \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    for (int i = 0; i < indent; i++) {                                         \
      trace_printf("  ");                                                      \
    }                                                                          \
    trace_printf(__VA_ARGS__);                                                 \
    trace_printf(" \e[1;36m[span_id=%lld]\e[0m",                               \
                 get_trace_ctx()->span_ids[get_trace_ctx()->active_span]);     \
    trace_printf("\n");                                                        \
  } while (0)

#define trace(...) trace_indent(get_trace_ctx()->active_span, __VA_ARGS__)

#define start_trace(trace_id, span_id, ...)                                    \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    trace_ctx_t *ctx = get_trace_ctx();                                        \
    if (ctx->active_span >= TRACE_MAX_DEPTH) {                                 \
      ctx->exceed_len++;                                                       \
      fprintf(stderr, "Warning: Exceed max trace depth\n");                    \
      break;                                                                   \
    }                                                                          \
    ctx->active_span++;                                                        \
    ctx->span_ids[ctx->active_span] = span_id;                                 \
    ctx->trace_ids[ctx->active_span] = trace_id;                               \
                                                                               \
    struct timespec ts;                                                        \
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);                                   \
    ctx->span_starts[ctx->active_span] = timespec_ms(ts);                      \
    trace_indent(ctx->active_span - 1, __VA_ARGS__);                           \
  } while (0)

#define end_trace()                                                            \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    trace_ctx_t *ctx = get_trace_ctx();                                        \
                                                                               \
    if (ctx->exceed_len > 0) {                                                 \
      ctx->exceed_len--;                                                       \
      break;                                                                   \
    }                                                                          \
    struct timespec now;                                                       \
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);                                  \
                                                                               \
    uint64_t trace_id = ctx->trace_ids[ctx->active_span];                      \
    trace_latency_hist_t *hist = &ctx->out_hist[trace_id];                     \
                                                                               \
    uint64_t start_time = ctx->span_starts[ctx->active_span];                  \
    uint64_t end_time = timespec_ms(now);                                      \
    uint64_t time_diff = end_time - start_time;                                \
                                                                               \
    hist->time += time_diff;                                                   \
    hist->count++;                                                             \
                                                                               \
    ctx->active_span--;                                                        \
  } while (0)

#define flush_trace()                                                          \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    printf("%.*s", (int)get_trace_ctx()->span_cursor, get_trace_ctx()->out);   \
    get_trace_ctx()->span_cursor = 0;                                          \
  } while (0)

#define flush_trace_hist()                                                     \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    trace_ctx_t *ctx = get_trace_ctx();                                        \
    for (uint32_t i = 0; i < TRACE_MAX_ITEMS; i++) {                           \
      trace_latency_hist_t *hist = &ctx->out_hist[i];                          \
      if (hist->count == 0 || hist->time == 0) {                               \
        continue;                                                              \
      }                                                                        \
      printf("Trace %d: %lld executions, avg time %.6f microseconds\n", i,     \
             hist->count, (float)hist->time / (float)hist->count);             \
      hist->count = 0;                                                         \
      hist->time = 0;                                                          \
    }                                                                          \
  } while (0)

#define sflush_trace(v_out, v_out_len)                                         \
  ({                                                                           \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    int written =                                                              \
        snprintf(v_out, v_out_len, "%.*s", (int)get_trace_ctx()->span_cursor,  \
                 get_trace_ctx()->out);                                        \
    get_trace_ctx()->span_cursor = 0;                                          \
    written;                                                                   \
  })

#define set_trace_span(span_id)                                                \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    get_trace_ctx()->span_ids[get_trace_ctx()->active_span] = span_id;         \
  } while (0)

#define trace_result(fmt, ...) "\e[1;32m=> " fmt "\e[0m"
#define trace_info(fmt, ...) "- " fmt
#define trace_span(fmt, ...) "\e[1;33m* " fmt "\e[0m"
#define trace_err(fmt, ...) "\e[1;31m!!! " fmt "\e[0m"

#endif // !TRACE_H
