#ifndef SET_DEBUG_H
#define SET_DEBUG_H

#include <stdbool.h>

#include "set.h"

size_t set_node_blackheight(set_node *nodes, uint8_t *colors, uint8_t *inited,
                            size_t start_node, bool is_start);
#endif // !SET_DEBUG_H
