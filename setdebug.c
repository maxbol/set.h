#include <stdbool.h>

#include "set.h"

size_t set_node_blackheight(set_node *nodes, uint8_t *colors, uint8_t *inited,
                            size_t start_node, bool is_start) {
  size_t node_idx = set_idx(start_node);
  set_node node = nodes[node_idx];
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

    size_t left_blackheight =
        set_node_blackheight(nodes, colors, inited, node.left, false);
    size_t right_blackheight =
        set_node_blackheight(nodes, colors, inited, node.right, false);

    if (left_blackheight != right_blackheight) {
      fprintf(stderr,
              "[Red/black violation error] Left and right blackheight of node "
              "with idx %zu is different (%zu "
              "vs %zu), this is not allowed in a red black tree. Panicking.\n",
              node_idx, left_blackheight, right_blackheight);
      exit(1);
    }
    counter += left_blackheight;
  }

  return counter;
}
