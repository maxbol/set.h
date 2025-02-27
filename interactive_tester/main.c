#include "set.h"
#include "setdebug.h"
#include "trace.h"
#include <locale.h>
#include <stdlib.h>

typedef set_type(uint32_t) set_t;

uint64_t hash_fn(uint32_t value) { return 1; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

typedef enum {
  Unknown = 0,
  ErroneousInput = 1,
  Add = 2,
  Remove = 3,
  Clear = 4,
  Print = 5,
  Back = 6,
  Next = 7,
} Command;

typedef struct {
  size_t line;
  size_t col;
} Cell;

#define TRACE_HIST_ITEM_BUF_SIZE 1024 * 1024

typedef struct {
  uint32_t len;
  char buf[TRACE_HIST_ITEM_BUF_SIZE];
} TraceHistItem;

#define commit_to_hist()                                                       \
  do {                                                                         \
    history_idx -= rewind_amount;                                              \
    rewind_limit = (rewind_limit - rewind_amount > 0)                          \
                       ? (rewind_limit - rewind_amount)                        \
                       : 0;                                                    \
    rewind_amount = 0;                                                         \
                                                                               \
    printf("Writing set to history slot %zu\n", history_idx + 1);              \
                                                                               \
    history_idx++;                                                             \
                                                                               \
    size_t history_cursor = history_idx % 5;                                   \
                                                                               \
    history[history_cursor] = set;                                             \
    trace_hist[history_cursor].len = sflush_trace(                             \
        trace_hist[history_cursor].buf, TRACE_HIST_ITEM_BUF_SIZE);             \
                                                                               \
    if (rewind_limit < 4) {                                                    \
      rewind_limit++;                                                          \
    }                                                                          \
  } while (0)

char *consume_opt(char *cmd_string, char *out) {
  unsigned int i = 0;

  while (true) {
    if (cmd_string[i] == ' ' || cmd_string[i] == '\n' || cmd_string[i] == 0) {
      break;
    }
    out[i] = cmd_string[i];
    i++;
  }

  return &cmd_string[i + 1];
}

char *read_cell(char *cmd_string, Command *cmd, Cell *cell) {
  char cmd_buf[64];
  memset(cmd_buf, 0, 64);

  cmd_string = consume_opt(cmd_string, cmd_buf);

  size_t len = strlen(cmd_buf);

  char parts[128][2] = {0};
  size_t part_idx = 0;
  uint32_t cursor = 0;

  if (strspn(cmd_buf, "0123456789,") != strlen(cmd_buf)) {
    fprintf(stderr, "Unrecognized characters in cell string: %s\n", cmd_buf);
    *cmd = ErroneousInput;
    return cmd_string;
  }

  for (uint32_t i = 0; i < len; i++) {
    if (cmd_buf[i] == ',') {
      if (part_idx == 0) {
        part_idx = 1;
        cursor = 0;
      } else {
        fprintf(stderr, "Unrecognized format of cell string: %s\n", cmd_buf);
        *cmd = ErroneousInput;
        return cmd_string;
      }
      continue;
    }
    parts[part_idx][cursor] = cmd_buf[i];
    cursor++;
  }

  *cell = (Cell){
      .line = atoi(parts[0]),
      .col = atoi(parts[1]),
  };

  return cmd_string;
}

char *read_entry(char *cmd_string, Command *cmd, uint32_t *entry) {
  char cmd_buf[64];
  memset(cmd_buf, 0, 64);

  cmd_string = consume_opt(cmd_string, cmd_buf);

  if (strspn(cmd_buf, "0123456789") == strlen(cmd_buf)) {
    *entry = atoi(cmd_buf);
  } else {
    // TODO(2025-02-20, Max Bolotin): Better error handling
    fprintf(stderr,
            "Error: Can't parse entry with non-numeric characters: <<%s>>\n",
            cmd_buf);
    *cmd = ErroneousInput;
  }

  return cmd_string;
}

char *read_command(char *cmd_string, Command *cmd, uint32_t *entry,
                   Cell *cell) {
  char cmd_buf[64];
  memset(cmd_buf, 0, 64);

  cmd_string = consume_opt(cmd_string, cmd_buf);

  if (strcmp(cmd_buf, "add") == 0) {
    *cmd = Add;
    cmd_string = read_entry(cmd_string, cmd, entry);
  } else if (strcmp(cmd_buf, "remove") == 0) {
    *cmd = Remove;
    cmd_string = read_entry(cmd_string, cmd, entry);
  } else if (strcmp(cmd_buf, "clear") == 0) {
    *cmd = Clear;
  } else if (strcmp(cmd_buf, "print") == 0) {
    *cmd = Print;
    cmd_string = read_cell(cmd_string, cmd, cell);
  } else if (strcmp(cmd_buf, "back") == 0) {
    *cmd = Back;
  } else if (strcmp(cmd_buf, "next") == 0) {
    *cmd = Next;
  } else {
    *cmd = Unknown;
  }

  return cmd_string;
}

int main(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  size_t canvas_width = 120;
  size_t canvas_height = 15;
  char *canvas = set_draw_alloc_canvas(canvas_width, canvas_height);

  const size_t out_buf_size = canvas_width * canvas_height * 64;
  char out_buf[out_buf_size];

  // Clear screen and reset cursor
  printf("\e[2J\e[H");

  set_t history[5];
  TraceHistItem trace_hist[5];
  size_t history_idx = 0;
  size_t rewind_amount = 0;
  size_t rewind_limit = 0;

  history[history_idx] = set;

  while (true) {
    assert(history_idx >= rewind_amount);

    size_t history_cursor = (history_idx - rewind_amount) % 5;

    set_t set = set_clone(history[history_cursor]);
    TraceHistItem trace_hist_item = trace_hist[history_cursor];

    printf("%.*s", trace_hist_item.len, trace_hist_item.buf);

    size_t blackheight = set_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, false);

    printf("Current blackheight: %zu\n", blackheight);

    set_clear_canvas(canvas, canvas_width, canvas_height);

    set_draw_tree_node(set.nodes, set.colors, set.inited, set.root, canvas,
                       canvas_width, canvas_height, 0, 0, ROOT);

    int written = set_draw_tree_canvas(canvas_width, canvas_height, canvas,
                                       out_buf, out_buf_size);

    printf("%.*s\n\n", written, out_buf);

    char command_buf[64];
    printf("Command: ");
    fgets(command_buf, sizeof(command_buf), stdin);

    // Clear screen and reset cursor
    printf("\e[2J\e[H");

    char *command_cursor = &command_buf[0];

    Command cmd;
    Cell cell;
    uint32_t entry;
    command_cursor = read_command(command_cursor, &cmd, &entry, &cell);

    switch (cmd) {
    case Unknown: {
      printf("Unknown command: %s", command_buf);
      continue;
    }
    case ErroneousInput: {
      continue;
    }
    case Add: {
      printf("Adding %d to set\n", entry);
      set_add(set, entry);
      commit_to_hist();
      break;
    }
    case Remove: {
      printf("Removing %d from set\n", entry);
      set_remove(set, entry);
      commit_to_hist();
      break;
    }
    case Clear: {
      printf("Clearing set\n");
      set_empty(set);
      commit_to_hist();
      break;
    }
    case Print: {
      size_t cell_idx = ((cell.line * canvas_width) + cell.col) * 64;
      char *cell_pointer = &canvas[cell_idx];
      printf("Cell contents:\n\n");
      for (uint32_t i = 0; i < 64; i++) {
        if (cell_pointer[i] == 0) {
          printf("\e[1;31m(N)\e[0m");
        } else if (cell_pointer[i] == '\e') {
          printf("\e[1;32m(Esc)\e[0m");
        } else {
          printf("%c", cell_pointer[i]);
        }
        if (i != 63) {
          printf("\e[1;33m|\e[0m");
        }
      }
      printf("\n\n\n");
      break;
    }
    case Back: {
      printf("Going back...\n");
      if (rewind_amount < rewind_limit) {
        rewind_amount++;
      } else {
        fprintf(stderr, "Can't go further backward\n");
      }
      break;
    }
    case Next: {
      if (rewind_amount > 0) {
        rewind_amount--;
      } else {
        fprintf(stderr, "Can't go further forward\n");
      }
      break;
    }
    }
  }

  free(canvas);
}
