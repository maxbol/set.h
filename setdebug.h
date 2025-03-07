#ifndef SET_DEBUG_H
#define SET_DEBUG_H

#include <stdbool.h>

#include "set.h"

#define CELL_SIZE 64

typedef enum { ROOT, LEFT, RIGHT } set_tree_line_alignment_t;

void debug_clear_canvas(char *canvas, size_t canvas_width,
                        size_t canvas_height);
int debug_draw_tree(tree_node_t *nodes, uint8_t *colors, uint8_t *inited,
                    size_t start_node, size_t canvas_width,
                    size_t canvas_height, char *out, size_t out_len);
int debug_draw_tree_canvas(size_t canvas_width, size_t canvas_height,
                           char *canvas, char *out, size_t out_len);
size_t debug_draw_tree_node(tree_node_t *nodes, uint8_t *colors,
                            uint8_t *inited, size_t start_node, char *canvas,
                            size_t canvas_width, size_t canvas_height,
                            size_t padding, size_t line,
                            set_tree_line_alignment_t alignment);
size_t debug_node_blackheight(tree_node_t *nodes, uint8_t *colors,
                              uint8_t *inited, size_t start_node, bool is_start,
                              bool assert_uniform_height);
char *debug_draw_alloc_canvas(size_t canvas_width, size_t canvas_height);
#endif // !SET_DEBUG_H
