#include <stdbool.h>

#include "set.h"
#include "setdebug.h"

void set_clear_canvas(char *canvas, size_t canvas_width, size_t canvas_height) {
  for (int i = 0; i < canvas_width * canvas_height; i++) {
    canvas[i * CELL_SIZE] = ' ';
    canvas[(i * CELL_SIZE) + 1] = 0;
  }
}

char *set_draw_alloc_canvas(size_t canvas_width, size_t canvas_height) {
  char *canvas = malloc(CELL_SIZE * canvas_width * canvas_height);
  set_clear_canvas(canvas, canvas_width, canvas_height);
  return canvas;
}

int set_draw_tree(set_node_t *nodes, uint8_t *colors, uint8_t *inited,
                  size_t start_node, size_t canvas_width, size_t canvas_height,
                  char *out, size_t out_len) {

  char *canvas = set_draw_alloc_canvas(canvas_width, canvas_height);

  set_draw_tree_node(nodes, colors, inited, start_node, canvas, canvas_width,
                     canvas_height, 0, 0, ROOT);

  size_t offset =
      set_draw_tree_canvas(canvas_width, canvas_height, canvas, out, out_len);

  free(canvas);

  return offset;
}

int set_draw_tree_canvas(size_t canvas_width, size_t canvas_height,
                         char *canvas, char *out, size_t out_len) {
  int offset = 0;

  for (size_t i = 0; i < canvas_width + 2; i++) {
    offset += snprintf(&out[offset], out_len - offset, "-");
  }
  offset += snprintf(&out[offset], out_len - offset, "\n");
  for (size_t i = 0; i < canvas_height; i++) {
    offset += snprintf(&out[offset], out_len - offset, "|");
    for (size_t j = 0; j < canvas_width; j++) {
      size_t cell = ((i * canvas_width) + j) * CELL_SIZE;
      offset += snprintf(&out[offset], out_len - offset, "%s", &canvas[cell]);
    }
    offset += snprintf(&out[offset], out_len - offset, "|");
    offset += snprintf(&out[offset], out_len - offset, "\n");
  }
  for (size_t i = 0; i < canvas_width + 2; i++) {
    offset += snprintf(&out[offset], out_len - offset, "-");
  }
  offset += snprintf(&out[offset], out_len - offset, "\n");

  return offset;
}

size_t set_draw_tree_node(set_node_t *nodes, uint8_t *colors, uint8_t *inited,
                          size_t start_node, char *canvas, size_t canvas_width,
                          size_t canvas_height, size_t padding, size_t line,
                          set_tree_line_alignment_t alignment) {
  assert(line < canvas_height);

  size_t node_idx = set_idx(start_node);
  set_node_t node = nodes[node_idx];

  size_t byte_idx = floor((float)node_idx / 8);
  size_t bit_idx = node_idx % 8;
  uint8_t mask = 1 << (7 - bit_idx);

  bool is_inited = (inited[byte_idx] & mask) != 0;
  bool is_red = (colors[byte_idx] & mask) != 0;

  size_t left_pad = 0;
  size_t right_pad = 0;

  if (node.left != 0) {
    if (node.right == 0) {
      fprintf(stderr,
              "Left child is non-zero but right child is zero for node %zu\n",
              node_idx);
    }
    assert(node.right != 0);

    size_t next_padding = set_draw_tree_node(
        nodes, colors, inited, node.left, canvas, canvas_width, canvas_height,
        padding, line + 2, LEFT);
    left_pad = next_padding - padding;
    padding = next_padding;
  }

  if (alignment == RIGHT) {
    size_t cell =
        ((canvas_width * (line - 1)) + padding - (left_pad > 0 ? 1 : 0)) *
        CELL_SIZE;

    canvas[cell] = '\\';

    for (int i = 1; i < left_pad; i++) {
      sprintf(&canvas[cell - (i * CELL_SIZE)], "¯");
    }
  }
  assert(padding < canvas_width);

  size_t cell = ((canvas_width * line) + padding) * CELL_SIZE;
  int data_len;
  if (is_inited) {
    char data_out[8];
    data_len = snprintf(data_out, 8, "%lld", node.hash);

    for (int i = 0; i < data_len; i++) {
      size_t cell = ((canvas_width * line) + padding) * CELL_SIZE;
      int offset = 0;
      if (i == 0) {
        if (is_red) {
          offset += sprintf(&canvas[cell + offset], "\e[1;31m");
        } else {
          offset += sprintf(&canvas[cell + offset], "\e[1m");
        }
      }
      canvas[cell + (offset++)] = data_out[i];
      if (i == data_len - 1) {
        offset += sprintf(&canvas[cell + offset], "\e[0m");
      }
      canvas[cell + offset] = 0;
      padding++;
    }
  } else {
    canvas[cell] = 'N';
    canvas[cell + 1] = 0;
    padding++;
    data_len = 1;
  }

  if (node.right != 0) {
    if (node.left == 0) {
      fprintf(stderr,
              "Right child is non-zero but left child is zero for node %zu\n",
              node_idx);
    }
    assert(node.left != 0);
    size_t next_padding = set_draw_tree_node(
        nodes, colors, inited, node.right, canvas, canvas_width, canvas_height,
        padding, line + 2, RIGHT);
    right_pad = next_padding - padding;
    padding = next_padding;
  }

  if (alignment == LEFT) {
    size_t cell = ((canvas_width * (line - 1)) + padding - right_pad -
                   data_len + (right_pad > 0 ? 1 : 0)) *
                  CELL_SIZE;

    canvas[cell] = '/';

    for (int i = 1; i < right_pad; i++) {
      sprintf(&canvas[cell + (i * CELL_SIZE)], "¯");
    }
  }

  return padding;
}
size_t set_node_blackheight(set_node_t *nodes, uint8_t *colors, uint8_t *inited,
                            size_t start_node, bool is_start,
                            bool assert_uniform_height) {
  size_t node_idx = set_idx(start_node);
  set_node_t node = nodes[node_idx];
  size_t counter = 0;

  size_t byte_idx = floor((float)node_idx / 8);
  size_t bit_idx = node_idx % 8;
  uint8_t mask = 1 << (7 - bit_idx);
  bool is_black = (colors[byte_idx] & mask) == 0;
  bool is_inited = (inited[byte_idx] & mask) != 0;

  if (!is_start && is_black) {
    counter += 1;
  }

  if (is_inited) {
    assert(node.left != 0);
    assert(node.right != 0);

    size_t left_blackheight = set_node_blackheight(
        nodes, colors, inited, node.left, false, assert_uniform_height);
    size_t right_blackheight = set_node_blackheight(
        nodes, colors, inited, node.right, false, assert_uniform_height);

    if (left_blackheight != right_blackheight) {
      fprintf(stderr,
              "Warning: Different blackheight discovered for children of node "
              "%zu (%lld) - left %zu, right %zu\n",
              node_idx, node.hash, left_blackheight, right_blackheight);
    }
    if (assert_uniform_height) {
      assert(left_blackheight == right_blackheight);
    }

    counter += left_blackheight;
  }

  return counter;
}
