#ifndef SET_DEBUG_H
#define SET_DEBUG_H

#include <stdbool.h>

#include "set.h"

typedef enum { ROOT, LEFT, RIGHT } set_tree_line_alignment_t;

size_t set_node_blackheight(set_node *nodes, uint8_t *colors, uint8_t *inited,
                            size_t start_node, bool is_start);
size_t set_draw_tree_node(set_node *nodes, uint8_t *colors, uint8_t *inited,
                          size_t start_node, char *canvas, size_t canvas_width,
                          size_t canvas_height, size_t padding, size_t line,
                          set_tree_line_alignment_t alignment);
int set_draw_tree(set_node *nodes, uint8_t *colors, uint8_t *inited,
                  size_t start_node, size_t canvas_width, size_t canvas_height,
                  char *out, size_t out_len);
char *set_draw_alloc_canvas(size_t canvas_width, size_t canvas_height);
#endif // !SET_DEBUG_H
