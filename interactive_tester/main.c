#include "set.h"
#include "setdebug.h"
#include <locale.h>
#include <stdlib.h>

typedef set_type(uint32_t) set_uint32_t;

uint64_t set_hash_uint32(uint32_t value) { return value; }

typedef enum {
  Unknown = 0,
  Add = 1,
  Remove = 2,
} Command;

char *read_entry(char *cmd_string, uint32_t *entry) {
  unsigned int i = 0;
  char cmd_buf[64];
  memset(cmd_buf, 0, 64);

  while (true) {
    if (cmd_string[i] == ' ' || cmd_string[i] == '\n' || cmd_string[i] == 0) {
      break;
    }
    cmd_buf[i] = cmd_string[i];
    i++;
  }
  i++;

  if (strspn(cmd_buf, "0123456789") == strlen(cmd_buf)) {
    *entry = atoi(cmd_buf);
  } else {
    // TODO(2025-02-20, Max Bolotin): Better error handling
    printf("strspn: %zu\n", strspn(cmd_buf, "0123456789"));
    printf("strlen: %zu\n", strlen(cmd_buf));
    fprintf(stderr,
            "Error: Can't parse entry with non-numeric characters: <<%s>>\n",
            cmd_buf);
    exit(1);
  }

  return &cmd_string[i];
}

char *read_command(char *cmd_string, Command *cmd, uint32_t *entry) {
  unsigned int i = 0;
  char cmd_buf[64];
  memset(cmd_buf, 0, 64);

  while (true) {
    if (cmd_string[i] == ' ' || cmd_string[i] == 0) {
      break;
    }
    cmd_buf[i] = cmd_string[i];
    i++;
  }
  i++;

  cmd_string = &cmd_string[i];

  if (strcmp(cmd_buf, "add") == 0) {
    *cmd = Add;
    cmd_string = read_entry(cmd_string, entry);
  } else if (strcmp(cmd_buf, "remove") == 0) {
    *cmd = Remove;
    cmd_string = read_entry(cmd_string, entry);
  } else {
    *cmd = Unknown;
  }

  return cmd_string;
}

int main(void) {
  set_uint32_t set;
  set_init(set, set_hash_uint32);

  size_t canvas_width = 120;
  size_t canvas_height = 26;

  char out_buf[canvas_width * canvas_height * 64];

  while (true) {
    int written = set_draw_tree(set.nodes, set.colors, set.inited, set.root,
                                canvas_width, canvas_height, out_buf,
                                canvas_width * canvas_height * 64);

    printf("%.*s\n\n", written, out_buf);

    char command_buf[64];
    printf("Command: ");
    fgets(command_buf, sizeof(command_buf), stdin);

    // Clear screen and reset cursor
    printf("\e[2J\e[H");

    char *command_cursor = &command_buf[0];

    Command cmd;
    uint32_t entry;
    command_cursor = read_command(command_cursor, &cmd, &entry);

    switch (cmd) {
    case Unknown: {
      printf("Unknown command: %s", command_buf);
      continue;
    }
    case Add: {
      printf("Adding %d to set\n", entry);
      set_add(set, entry);
      break;
    }
    case Remove: {
      printf("Removing %d from set\n", entry);
      set_remove(set, entry);
      break;
    }
    }

    set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true);
  }
}
