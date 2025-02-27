#ifndef SET_DEBUG_H
#define SET_DEBUG_H

#include <stdbool.h>

#include "set.h"

#define CELL_SIZE 64

typedef enum { ROOT, LEFT, RIGHT } set_tree_line_alignment_t;

void set_clear_canvas(char *canvas, size_t canvas_width, size_t canvas_height);
int set_draw_tree(set_node_t *nodes, uint8_t *colors, uint8_t *inited,
                  size_t start_node, size_t canvas_width, size_t canvas_height,
                  char *out, size_t out_len);
int set_draw_tree_canvas(size_t canvas_width, size_t canvas_height,
                         char *canvas, char *out, size_t out_len);
size_t set_draw_tree_node(set_node_t *nodes, uint8_t *colors, uint8_t *inited,
                          size_t start_node, char *canvas, size_t canvas_width,
                          size_t canvas_height, size_t padding, size_t line,
                          set_tree_line_alignment_t alignment);
size_t set_node_blackheight(set_node_t *nodes, uint8_t *colors, uint8_t *inited,
                            size_t start_node, bool is_start,
                            bool assert_uniform_height);
char *set_draw_alloc_canvas(size_t canvas_width, size_t canvas_height);
#endif // !SET_DEBUG_H
