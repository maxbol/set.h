#ifndef TRACE_H
#define TRACE_H

#include <stdlib.h>

typedef struct {
  uint32_t active_span;
  char *out;
  size_t cursor;
  uint64_t span_ids[1024];
} trace_ctx_t;

trace_ctx_t *get_trace_ctx();

#define trace_printf(...)                                                      \
  get_trace_ctx()->cursor +=                                                   \
      sprintf(&get_trace_ctx()->out[get_trace_ctx()->cursor], __VA_ARGS__)

#define trace_indent(indent, ...)                                              \
  do {                                                                         \
    for (int i = 0; i < indent; i++) {                                         \
      trace_printf(" ");                                                       \
    }                                                                          \
    trace_printf(__VA_ARGS__);                                                 \
    trace_printf(" \e[1;36m[span_id=%lld]\e[0m",                               \
                 get_trace_ctx()->span_ids[get_trace_ctx()->active_span]);     \
    trace_printf("\n");                                                        \
  } while (0)

#define trace(...) trace_indent(get_trace_ctx()->active_span, __VA_ARGS__)

#define start_trace(span_id, ...)                                              \
  do {                                                                         \
    get_trace_ctx()->span_ids[++get_trace_ctx()->active_span] = span_id;       \
    trace_indent(get_trace_ctx()->active_span - 1, __VA_ARGS__);               \
  } while (0)

#define end_trace()                                                            \
  do {                                                                         \
    get_trace_ctx()->active_span--;                                            \
  } while (0)

#define flush_trace()                                                          \
  do {                                                                         \
    printf("%.*s", (int)get_trace_ctx()->cursor, get_trace_ctx()->out);        \
    get_trace_ctx()->cursor = 0;                                               \
  } while (0)

#define set_trace_span(span_id)                                                \
  do {                                                                         \
    get_trace_ctx()->span_ids[get_trace_ctx()->active_span] = span_id;         \
  } while (0)

#define trace_result(fmt, ...) "\e[1;32m=> " fmt "\e[0m"
#define trace_info(fmt, ...) "- " fmt
#define trace_span(fmt, ...) "\e[1;33m* " fmt "\e[0m"
#define trace_err(fmt, ...) "\e[1;31m!!! " fmt "\e[0m"

#endif // !TRACE_H
