#ifndef TRACE_H
#define TRACE_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  uint32_t active_span;
  char *out;
  size_t cursor;
  uint64_t span_ids[1024];
} trace_ctx_t;

extern trace_ctx_t *get_trace_ctx();
extern void enable_tracing();
extern void disable_tracing();
extern bool tracing_enabled;

#define trace_printf(...)                                                      \
  get_trace_ctx()->cursor +=                                                   \
      sprintf(&get_trace_ctx()->out[get_trace_ctx()->cursor], __VA_ARGS__)

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

#define start_trace(span_id, ...)                                              \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    get_trace_ctx()->span_ids[++get_trace_ctx()->active_span] = span_id;       \
    trace_indent(get_trace_ctx()->active_span - 1, __VA_ARGS__);               \
  } while (0)

#define end_trace()                                                            \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    get_trace_ctx()->active_span--;                                            \
  } while (0)

#define flush_trace()                                                          \
  do {                                                                         \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    printf("%.*s", (int)get_trace_ctx()->cursor, get_trace_ctx()->out);        \
    get_trace_ctx()->cursor = 0;                                               \
  } while (0)

#define sflush_trace(v_out, v_out_len)                                         \
  ({                                                                           \
    if (!tracing_enabled)                                                      \
      break;                                                                   \
    int written =                                                              \
        snprintf(v_out, v_out_len, "%.*s", (int)get_trace_ctx()->cursor,       \
                 get_trace_ctx()->out);                                        \
    get_trace_ctx()->cursor = 0;                                               \
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
