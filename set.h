#ifndef GENERIC_SET_H
#define GENERIC_SET_H

#ifndef SET_TRACE_STEPS
#define SET_TRACE_STEPS false
#endif // !SET_TRACE_STEPS

#if SET_TRACE_STEPS
#include "trace.h"
#else
#define trace_noop(...) (__VA_ARGS__)
#define trace(...) trace_noop(__VA_ARGS__)
#define start_trace(...) trace_noop(__VA_ARGS__)
#define end_trace()
#define flush_trace()
#define set_trace_span(...) trace_noop(__VA_ARGS__)
#define trace_result(...) trace_noop(__VA_ARGS__)
#define trace_info(...) trace_noop(__VA_ARGS__)
#define trace_span(...) trace_noop(__VA_ARGS__)
#define trace_err(...) trace_noop(__VA_ARGS__)
#endif

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_NIL 0
#define IDX_NIL 0
#define NODE_COLOR_BLACK 0
#define NODE_COLOR_RED 1
#define NODE_UNINITED 0
#define NODE_INITED 1

#define NODE_NIL                                                               \
  {                                                                            \
      .hash = HASH_NIL,                                                        \
      .left = IDX_NIL,                                                         \
      .right = IDX_NIL,                                                        \
  };

typedef struct {
  size_t left;
  size_t right;
  size_t parent;
  uint64_t hash;
} set_node;

#define SET_ALLOC_CHUNK 512

#define BIT_HEADER_ADDR ((size_t)1 << 63)
#define BIT_HEADER_IDX (~BIT_HEADER_ADDR)

#define set_add(set, entry_var)                                                \
  do {                                                                         \
    uint64_t hash = set.hash_fn(entry_var);                                    \
    start_trace(hash, trace_span("Adding node"));                              \
    size_t leaf_idx = set_find_node(set, hash);                                \
                                                                               \
    if (set_is_inited(set, leaf_idx) != 0) {                                   \
      end_trace();                                                             \
      break;                                                                   \
    }                                                                          \
                                                                               \
    typeof(set.nodes) leaf = set_get_node(set, leaf_idx);                      \
                                                                               \
    size_t left_idx = set_alloc_new_node(set);                                 \
    size_t right_idx = set_alloc_new_node(set);                                \
    set_get_node(set, left_idx)->parent = leaf_idx;                            \
    set_get_node(set, right_idx)->parent = leaf_idx;                           \
    set_write_color(set, left_idx, NODE_COLOR_BLACK);                          \
    set_write_color(set, right_idx, NODE_COLOR_BLACK);                         \
                                                                               \
    *leaf = (typeof(*set.nodes)){                                              \
        .hash = hash,                                                          \
        .left = left_idx,                                                      \
        .parent = leaf->parent,                                                \
        .right = right_idx,                                                    \
    };                                                                         \
                                                                               \
    set_write_entry(set, leaf_idx, entry_var);                                 \
    set_write_color(set, leaf_idx, NODE_COLOR_RED);                            \
                                                                               \
    set_rb_insert_fixup(set, leaf_idx);                                        \
    end_trace();                                                               \
  } while (0)

#define set_addr(idx) (idx) + 1
#define set_alloc_new_node(set)                                                \
  ({                                                                           \
    size_t retval;                                                             \
    bool found_free = false;                                                   \
    for (size_t i = 0; i < set.capacity; i++) {                                \
      size_t byte_idx = floor((float)i / 8);                                   \
      uint8_t bit_idx = i % 8;                                                 \
      uint8_t bmask = 1 << (7 - bit_idx);                                      \
      if ((set.free_list[byte_idx] & bmask) != 0) {                            \
        found_free = true;                                                     \
        set.free_list[byte_idx] ^= bmask;                                      \
        set.nodes[i] = (typeof(*set.nodes))NODE_NIL;                           \
        memset(&set.entries[i], 0x00, sizeof(typeof(*set.entries)));           \
        retval = set_addr(i);                                                  \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if (!found_free) {                                                         \
      size_t i = set.capacity;                                                 \
      retval = set_addr(i);                                                    \
      set.capacity *= 2;                                                       \
      set.nodes =                                                              \
          realloc(set.nodes, sizeof(typeof(*set.nodes)) * set.capacity);       \
      set.entries =                                                            \
          realloc(set.entries, sizeof(typeof(*set.entries)) * set.capacity);   \
      set.free_list = realloc(set.free_list, set.capacity / 8);                \
      set.colors = realloc(set.colors, set.capacity / 8);                      \
      set.inited = realloc(set.inited, set.capacity / 8);                      \
      memset(&set.free_list[i / 8], 0xff, (set.capacity / 8) - (i / 8));       \
      memset(&set.colors[i / 8], 0x00, (set.capacity / 8) - (i / 8));          \
      memset(&set.inited[i / 8], 0x00, (set.capacity / 8) - (i / 8));          \
      set.nodes[i] = (typeof(*set.nodes))NODE_NIL;                             \
      memset(&set.entries[i], 0x00, sizeof(typeof(*set.entries)));             \
      set.free_list[i] = 0xff >> 1;                                            \
    }                                                                          \
    retval;                                                                    \
  })

#define set_clear_entry(set, addr)                                             \
  do {                                                                         \
    size_t clear_idx = set_idx(addr);                                          \
    assert(set.capacity > clear_idx);                                          \
    memset(&set.entries[clear_idx], 0x00, sizeof(typeof(set.entries)));        \
    set_write_inited(set, addr, false);                                        \
  } while (0)

#define set_clone(set)                                                         \
  ({                                                                           \
    typeof(set) clone;                                                         \
    typeof(set.hash_fn) hash_function = set.hash_fn;                           \
                                                                               \
    clone.capacity = set.capacity;                                             \
    clone.root = set.root;                                                     \
    clone.nodes = malloc(sizeof(typeof(*clone.nodes)) * set.capacity);         \
    clone.entries = malloc(sizeof(typeof(*clone.entries)) * set.capacity);     \
    clone.free_list = malloc(clone.capacity / 8);                              \
    clone.colors = malloc(clone.capacity / 8);                                 \
    clone.inited = malloc(clone.capacity / 8);                                 \
                                                                               \
    for (size_t i = 0; i < clone.capacity; i++) {                              \
      clone.nodes[i] = set.nodes[i];                                           \
      clone.entries[i] = set.entries[i];                                       \
    }                                                                          \
    for (size_t i = 0; i < clone.capacity / 8; i++) {                          \
      clone.free_list[i] = set.free_list[i];                                   \
      clone.colors[i] = set.colors[i];                                         \
      clone.inited[i] = set.inited[i];                                         \
    }                                                                          \
    clone.hash_fn = hash_function;                                             \
                                                                               \
    clone;                                                                     \
  })

#define set_delete_fixup(set, node_idx)                                        \
  do {                                                                         \
    while (node_idx != set.root) {                                             \
      typeof(set.nodes) node = set_get_node(set, node_idx);                    \
      start_trace(node->hash, trace_span("Fixing up %lld"), node->hash);       \
      bool color = set_is_red(set, node_idx);                                  \
      if (color != NODE_COLOR_BLACK) {                                         \
        trace(trace_info("Node is red, stop traversing"));                     \
        end_trace();                                                           \
        break;                                                                 \
      }                                                                        \
      if (node == set_get_sibling(set, node, left)) {                          \
        trace(trace_info("Node is in parents left subtree"));                  \
        set_delete_fixup_dir(set, node_idx, right, left);                      \
      } else {                                                                 \
        set_delete_fixup_dir(set, node_idx, left, right);                      \
      }                                                                        \
      end_trace();                                                             \
    }                                                                          \
    set_write_color(set, node_idx, NODE_COLOR_BLACK);                          \
  } while (0)

#define set_delete_fixup_dir(set, node_idx, f_branch, f_direction)             \
  do {                                                                         \
    typeof(set.nodes) node = set_get_node(set, node_idx);                      \
    typeof(set.nodes) parent = set_get_node(set, node->parent);                \
    size_t sibling_idx = parent->f_branch;                                     \
                                                                               \
    if (set_is_red(set, sibling_idx) == NODE_COLOR_RED) {                      \
      set_write_color(set, sibling_idx, NODE_COLOR_BLACK);                     \
      set_write_color(set, node->parent, NODE_COLOR_RED);                      \
      set_rot(set, node->parent, f_branch, f_direction);                       \
      sibling_idx = set_get_node(set, node->parent)->f_branch;                 \
    }                                                                          \
                                                                               \
    typeof(set.nodes) sibling = set_get_node(set, sibling_idx);                \
                                                                               \
    if (set_is_red(set, sibling->f_branch) == NODE_COLOR_BLACK &&              \
        set_is_red(set, sibling->f_direction) == NODE_COLOR_BLACK) {           \
      set_write_color(set, sibling_idx, NODE_COLOR_RED);                       \
      node_idx = node->parent;                                                 \
      node = set_get_node(set, node_idx);                                      \
    } else {                                                                   \
      if (set_is_red(set, sibling->f_branch) == NODE_COLOR_BLACK) {            \
        set_write_color(set, sibling->f_direction, NODE_COLOR_BLACK);          \
        set_write_color(set, sibling_idx, NODE_COLOR_RED);                     \
        set_rot(set, sibling_idx, f_direction, f_branch);                      \
        sibling_idx =                                                          \
            set_get_node(set, set_get_node(set, node_idx)->parent)->f_branch;  \
        sibling = set_get_node(set, sibling_idx);                              \
      }                                                                        \
      set_write_color(set, sibling_idx, set_is_red(set, node->parent));        \
      set_write_color(set, node->parent, NODE_COLOR_BLACK);                    \
      set_write_color(set, sibling->f_branch, NODE_COLOR_BLACK);               \
      set_rot(set, node->parent, f_branch, f_direction);                       \
      node_idx = set.root;                                                     \
    }                                                                          \
  } while (0)

#define set_empty(set)                                                         \
  do {                                                                         \
    typeof(set.hash_fn) hash_fn = set.hash_fn;                                 \
    set_free(set);                                                             \
    set_init(set, hash_fn);                                                    \
  } while (0)

#define set_find_node(set, key)                                                \
  ({                                                                           \
    size_t n_idx = set.root;                                                   \
    size_t find_node_retval;                                                   \
    while (set_is_valid_addr(n_idx)) {                                         \
      find_node_retval = n_idx;                                                \
      typeof(set.nodes) node = set_get_node(set, n_idx);                       \
      if (key > node->hash) {                                                  \
        n_idx = node->right;                                                   \
      } else if (key < node->hash) {                                           \
        n_idx = node->left;                                                    \
      } else {                                                                 \
        break;                                                                 \
      }                                                                        \
    };                                                                         \
    find_node_retval;                                                          \
  })

#define set_first(set) set_ult(set, left)

#define set_free(set)                                                          \
  do {                                                                         \
    free(set.nodes);                                                           \
    free(set.entries);                                                         \
    free(set.colors);                                                          \
    free(set.inited);                                                          \
    free(set.free_list);                                                       \
  } while (0)

#define set_free_node(set, addr)                                               \
  do {                                                                         \
    size_t idx = set_idx(addr);                                                \
    size_t free_list_byte_idx = floor((float)idx / 8);                         \
    size_t free_list_bit_idx = idx % 8;                                        \
    uint8_t bmask = 1 << (7 - free_list_bit_idx);                              \
    set.free_list[free_list_byte_idx] ^= bmask;                                \
  } while (0)

#define set_get_entry(set, addr)                                               \
  ({                                                                           \
    size_t idx = set_idx(addr);                                                \
    assert(set.capacity > idx);                                                \
    set.entries[idx];                                                          \
  })

#define set_get_node(set, addr)                                                \
  ({                                                                           \
    size_t a = (addr);                                                         \
    typeof(set.nodes) retval = NULL;                                           \
    if (set_is_valid_addr(a)) {                                                \
      size_t idx = set_idx(a);                                                 \
      assert(set.capacity > idx);                                              \
      retval = &set.nodes[idx];                                                \
    }                                                                          \
    retval;                                                                    \
  })

#define set_get_sibling(set, node, f_branch)                                   \
  set_get_node(set, set_get_node(set, node->parent)->f_branch)

#define set_has(set, entry)                                                    \
  ({                                                                           \
    uint64_t hash = set.hash_fn(entry);                                        \
    size_t node_idx = set_find_node(set, hash);                                \
    typeof(set.nodes) node = set_get_node(set, node_idx);                      \
    set_is_inited(set, node_idx) && node->hash == hash;                        \
  })

#define set_idx(addr) (addr) - 1
#define set_init(set, hash_function)                                           \
  do {                                                                         \
    set.capacity = SET_ALLOC_CHUNK;                                            \
    set.nodes = malloc(sizeof(typeof(*set.nodes)) * set.capacity);             \
    set.entries = malloc(sizeof(typeof(*set.entries)) * set.capacity);         \
    set.free_list = malloc(set.capacity / 8);                                  \
    set.colors = malloc(set.capacity / 8);                                     \
    set.inited = malloc(set.capacity / 8);                                     \
    memset(set.free_list, 0xff, set.capacity / 8);                             \
    memset(set.colors, 0x00, set.capacity / 8);                                \
    memset(set.inited, 0x00, set.capacity / 8);                                \
    set.root = set_alloc_new_node(set);                                        \
    set.hash_fn = hash_function;                                               \
  } while (0)

#define set_is_inited(set, addr) set_read_bitval(set, addr, inited)
#define set_is_red(set, addr) set_read_bitval(set, addr, colors)
#define set_is_valid_addr(addr) ((size_t)addr != 0)

#define set_last(set) set_ult(set, right)

#define set_max_in_bnrach(set, node_idx)                                       \
  set_ult_in_branch(set, node_idx, right);
#define set_min_in_branch(set, node_idx) set_ult_in_branch(set, node_idx, left);

#define set_next(set, node_idx) set_seq(set, node_idx, right, left)
#define set_next_in_branch(set, node_idx)                                      \
  set_seq_in_branch(set, node_idx, right, left);

#define set_prev(set, node_idx) set_seq(set, node_idx, left, right)
#define set_prev_in_branch(set, node_idx)                                      \
  set_seq_in_branch(set, node_idx, left, right);

#define set_rb_insert_fixup(set, node_idx)                                     \
  do {                                                                         \
    while (true) {                                                             \
      typeof(set.nodes) node = set_get_node(set, node_idx);                    \
      start_trace(node->hash, trace_span("Fixing up node %lld"), node->hash);  \
      typeof(set.nodes) parent = set_get_node(set, node->parent);              \
      if (parent == NULL || set_is_red(set, node->parent) != NODE_COLOR_RED) { \
        trace(trace_info("Parent is NULL or black, doing nothing"));           \
        end_trace();                                                           \
        break;                                                                 \
      }                                                                        \
      if (parent == set_get_sibling(set, parent, left)) {                      \
        trace(trace_info(                                                      \
            "Parent is in its parents left tree, doing fixup_left"));          \
        set_rb_insert_fixup_left(set, node_idx);                               \
      } else {                                                                 \
        trace(trace_info("Parent is in its parents right tree, "               \
                         "doing fixup_right"));                                \
        set_rb_insert_fixup_right(set, node_idx);                              \
      }                                                                        \
      end_trace();                                                             \
    }                                                                          \
    start_trace(set_get_node(set, set.root)->hash,                             \
                trace_span("Setting root to black"));                          \
    set_write_color(set, set.root, NODE_COLOR_BLACK);                          \
    end_trace();                                                               \
  } while (0)

#define set_rb_insert_fixup_dir(set, node_idx, f_branch, f_direction)          \
  do {                                                                         \
    typeof(set.nodes) node = set_get_node(set, node_idx);                      \
    size_t parent_idx = node->parent;                                          \
                                                                               \
    typeof(set.nodes) parent = set_get_node(set, parent_idx);                  \
    size_t grandparent_idx = parent->parent;                                   \
                                                                               \
    typeof(set.nodes) grandparent = set_get_node(set, grandparent_idx);        \
    size_t uncle_idx = grandparent->f_branch;                                  \
                                                                               \
    if (set_is_red(set, uncle_idx) == NODE_COLOR_RED) {                        \
      start_trace(node->hash, trace_span("Uncle is red."));                    \
                                                                               \
      trace(trace_result("Setting parent to black."));                         \
      set_write_color(set, parent_idx, NODE_COLOR_BLACK);                      \
      trace(trace_result("Setting uncle to black."));                          \
      set_write_color(set, uncle_idx, NODE_COLOR_BLACK);                       \
      trace(trace_result("Setting grandparent to red."));                      \
      set_write_color(set, grandparent_idx, NODE_COLOR_RED);                   \
                                                                               \
      node_idx = grandparent_idx;                                              \
                                                                               \
      end_trace();                                                             \
    } else {                                                                   \
      start_trace(node->hash, trace_span("Uncle is black."));                  \
                                                                               \
      if (node == set_get_sibling(set, node, f_branch)) {                      \
        start_trace(                                                           \
            node->hash,                                                        \
            trace_span("Triangle case (node is aligned in the opposite way "   \
                       "as its parent)"));                                     \
                                                                               \
        node_idx = node->parent;                                               \
                                                                               \
        node = set_get_node(set, node_idx);                                    \
                                                                               \
        trace(trace_result("Updating node pointer to parent: %lld"),           \
              node->hash);                                                     \
        set_trace_span(node->hash);                                            \
        start_trace(node->hash, trace_span("Rotating node to " #f_direction)); \
                                                                               \
        set_rot(set, node_idx, f_branch, f_direction);                         \
                                                                               \
        parent_idx = node->parent;                                             \
                                                                               \
        parent = set_get_node(set, parent_idx);                                \
                                                                               \
        end_trace();                                                           \
        end_trace();                                                           \
      }                                                                        \
                                                                               \
      assert(parent->parent != 0);                                             \
                                                                               \
      start_trace(                                                             \
          node->hash,                                                          \
          trace_span("Line case (node is aligned same way as its parent)"));   \
                                                                               \
      trace(trace_result("Setting parent color to black"));                    \
                                                                               \
      set_write_color(set, parent_idx, NODE_COLOR_BLACK);                      \
                                                                               \
      trace(trace_result("Setting grandparent color to red"));                 \
                                                                               \
      set_write_color(set, parent->parent, NODE_COLOR_RED);                    \
                                                                               \
      start_trace(set_get_node(set, parent->parent)->hash,                     \
                  trace_span("Rotating grandparent of %lld to " #f_branch),    \
                  node->hash);                                                 \
                                                                               \
      set_rot(set, parent->parent, f_direction, f_branch);                     \
                                                                               \
      end_trace();                                                             \
      end_trace();                                                             \
    }                                                                          \
  } while (0)

#define set_rb_insert_fixup_left(set, node_idx)                                \
  set_rb_insert_fixup_dir(set, node_idx, right, left)
#define set_rb_insert_fixup_right(set, node_idx)                               \
  set_rb_insert_fixup_dir(set, node_idx, left, right)

#define set_read_bitval(set, addr, f_member)                                   \
  ({                                                                           \
    size_t idx = set_idx(addr);                                                \
    size_t byte_idx = floor((float)idx / 8);                                   \
    size_t bit_idx = idx % 8;                                                  \
    uint8_t mask = 1 << (7 - bit_idx);                                         \
    (set.f_member[byte_idx] & mask) != 0;                                      \
  })

// TODO(2025-02-26, Max Bolotin): To reproduce - add, 100, 200, 300, 400, 500
// 600, remove 600, 400
#define set_remove(set, entry)                                                 \
  do {                                                                         \
    uint64_t hash = set.hash_fn(entry);                                        \
    start_trace(hash, trace_span("Removing entry %lld"), hash);                \
    size_t node_idx = set_find_node(set, hash);                                \
    typeof(set.nodes) node = set_get_node(set, node_idx);                      \
                                                                               \
    if (node->hash != hash) {                                                  \
      trace(trace_info("Entry does not exist in tree"));                       \
      end_trace();                                                             \
      break;                                                                   \
    }                                                                          \
                                                                               \
    size_t color_sample_idx = node_idx;                                        \
    size_t fixup_target_idx;                                                   \
                                                                               \
    bool color_sample_is_red = set_is_red(set, node_idx);                      \
                                                                               \
    if (!set_is_inited(set, node->left)) {                                     \
      trace(trace_info(                                                        \
          "Left child is NIL node or both children are NIL nodes"));           \
      typeof(set.nodes) right = set_get_node(set, node->right);                \
      trace(trace_result("Setting fixup target to right child: %lld"),         \
            right->hash);                                                      \
      fixup_target_idx = node->right;                                          \
      start_trace(node_idx,                                                    \
                  trace_span("Transplanting right child (node %lld) here"),    \
                  right->hash);                                                \
      set_transplant(set, node_idx, node->right);                              \
      end_trace();                                                             \
    } else if (!set_is_inited(set, node->right)) {                             \
      trace(trace_info("Only right child is NIL node"));                       \
      typeof(set.nodes) left = set_get_node(set, node->left);                  \
      trace(trace_result("Setting fixup target to left child: %lld"),          \
            left->hash);                                                       \
      fixup_target_idx = node->left;                                           \
      start_trace(node_idx,                                                    \
                  trace_span("Transplanting left child (node %lld) here"),     \
                  left->hash);                                                 \
      set_transplant(set, node_idx, node->left);                               \
      end_trace();                                                             \
    } else {                                                                   \
      trace(trace_info("Neither of children are NIL nodes"));                  \
      assert(set_is_inited(set, node->right));                                 \
      color_sample_idx = set_min_in_branch(set, node->right);                  \
      typeof(set.nodes) color_sample = set_get_node(set, color_sample_idx);    \
      color_sample_is_red = set_is_red(set, color_sample_idx);                 \
      trace(trace_info("Found color sample as minimum of right child: %lld - " \
                       "color is %s"),                                         \
            color_sample->hash, color_sample_is_red ? "red" : "black");        \
      fixup_target_idx = color_sample->right;                                  \
      typeof(set.nodes) fixup_target = set_get_node(set, fixup_target_idx);    \
      trace(trace_result(                                                      \
                "Setting fixup target to right child of color sample: %lld"),  \
            fixup_target->hash);                                               \
                                                                               \
      if (color_sample_idx == node->right) {                                   \
        trace(trace_info("Color sample is right child of node"));              \
        trace(                                                                 \
            trace_result(                                                      \
                "Setting fixup target (%lld) parent to color sample (%lld)"),  \
            fixup_target->hash, color_sample->hash);                           \
        fixup_target->parent = color_sample_idx;                               \
      } else {                                                                 \
        trace(trace_info("Color sample is not right child of node"));          \
        start_trace(color_sample->hash,                                        \
                    trace_span("Transplanting %lld to color sample (%lld)"),   \
                    fixup_target->hash, color_sample->hash);                   \
        set_transplant(set, color_sample_idx, fixup_target_idx);               \
        end_trace();                                                           \
        trace(trace_result(                                                    \
                  "2-way binding right child of color sample (%lld) to "       \
                  "right child of node (%lld)"),                               \
              color_sample->hash, node->hash);                                 \
        color_sample->right = node->right;                                     \
        set_get_node(set, color_sample->right)->parent = color_sample_idx;     \
      }                                                                        \
                                                                               \
      start_trace(                                                             \
          node->hash,                                                          \
          trace_span("Transplanting color sample (%lld) to node (%lld)"),      \
          color_sample->hash, node->hash);                                     \
      set_transplant(set, node_idx, color_sample_idx);                         \
      end_trace();                                                             \
                                                                               \
      trace(trace_result(                                                      \
                "2-way binding left child of color sample (%lld) to left "     \
                "child of node (%lld)"),                                       \
            color_sample->hash, node->hash);                                   \
      color_sample->left = node->left;                                         \
      set_get_node(set, color_sample->left)->parent = color_sample_idx;        \
      trace(trace_result("Setting color of color sample (%lld) to %s"),        \
            color_sample->hash, set_is_red(set, node_idx) ? "red" : "black");  \
      set_write_color(set, color_sample_idx, set_is_red(set, node_idx));       \
    }                                                                          \
                                                                               \
    if (!color_sample_is_red) {                                                \
      trace(trace_info("Original color of color sample was black, fixing up "  \
                       "fix up target"));                                      \
      start_trace(fixup_target_idx, trace_span("Fixing up fixup target"));     \
      set_delete_fixup(set, fixup_target_idx);                                 \
      end_trace();                                                             \
    } else {                                                                   \
      trace(trace_info(                                                        \
          "Original color of color sample was red, no fixup required"));       \
    }                                                                          \
                                                                               \
    trace(trace_result("Freeing node"));                                       \
    set_free_node(set, node_idx);                                              \
    trace(trace_result("Clearing entry"));                                     \
    set_clear_entry(set, node_idx);                                            \
                                                                               \
    end_trace();                                                               \
  } while (0)

#define set_rot(set, node_idx, f_branch, f_direction)                          \
  do {                                                                         \
    size_t n_idx = (node_idx);                                                 \
    const char *align_branch = #f_branch;                                      \
    const char *align_direction = #f_direction;                                \
    typeof(set.nodes) rot_node = set_get_node(set, n_idx);                     \
    size_t f_branch_idx = rot_node->f_branch;                                  \
    assert(set_is_valid_addr(f_branch_idx));                                   \
    typeof(set.nodes) f_branch = set_get_node(set, f_branch_idx);              \
                                                                               \
    trace(trace_result("Binding %s field to %s child of %s child"),            \
          align_branch, align_direction, align_branch);                        \
    rot_node->f_branch = f_branch->f_direction;                                \
    if (set_is_valid_addr(f_branch->f_direction)) {                            \
      set_get_node(set, f_branch->f_direction)->parent = n_idx;                \
    }                                                                          \
                                                                               \
    f_branch->parent = rot_node->parent;                                       \
    if (!set_is_valid_addr(rot_node->parent)) {                                \
      trace(trace_result("Binding root to %s child"), align_branch);           \
      set.root = f_branch_idx;                                                 \
    } else if (rot_node == set_get_sibling(set, rot_node, f_direction)) {      \
      trace(trace_result("Binding %s field of parent to %s child"),            \
            align_direction, align_branch);                                    \
      set_get_node(set, rot_node->parent)->f_direction = f_branch_idx;         \
    } else {                                                                   \
      trace(trace_result("Binding %s field of parent to %s child"),            \
            align_branch, align_branch);                                       \
      set_get_node(set, rot_node->parent)->f_branch = f_branch_idx;            \
    }                                                                          \
                                                                               \
    trace(trace_result("Binding %s field of %s child to this node"),           \
          align_direction, align_branch);                                      \
    f_branch->f_direction = n_idx;                                             \
    rot_node->parent = f_branch_idx;                                           \
  } while (0)

#define set_rot_left(set, node_idx) set_rot(set, node_idx, right, left)
#define set_rot_right(set, node_idx) set_rot(set, node_idx, left, right)

#define set_seq(set, node_idx, f_branch, f_direction)                          \
  ({                                                                           \
    size_t addr = set_seq_in_branch(set, node_idx, f_branch, f_direction);     \
                                                                               \
    if (!set_is_valid_addr(addr)) {                                            \
      size_t scan_addr = node_idx;                                             \
      size_t next = set_get_node(set, node_idx)->parent;                       \
      while (set_is_valid_addr(next)) {                                        \
        typeof(set.nodes) next_node = set_get_node(set, next);                 \
        if (next_node->f_direction == scan_addr) {                             \
          addr = next;                                                         \
          break;                                                               \
        }                                                                      \
        scan_addr = next;                                                      \
        next = next_node->parent;                                              \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (!set_is_valid_addr(addr)) {                                            \
      addr = 0;                                                                \
    }                                                                          \
                                                                               \
    addr;                                                                      \
  })

#define set_seq_in_branch(set, node_idx, f_branch, f_direction)                \
  ({                                                                           \
    size_t idx = 0;                                                            \
    if (set_is_inited(set, node_idx) == 1) {                                   \
      typeof(set.nodes) node = set_get_node(set, node_idx);                    \
                                                                               \
      if (set_is_inited(set, node->f_branch) == 1) {                           \
        idx = set_ult_in_branch(set, node->f_branch, f_direction);             \
      }                                                                        \
    }                                                                          \
    idx;                                                                       \
  })

#define set_size(set)                                                          \
  ({                                                                           \
    size_t cursor = set_first(set);                                            \
    size_t size = 0;                                                           \
    while (set_is_valid_addr(cursor) && set_is_inited(set, cursor)) {          \
      size++;                                                                  \
      cursor = set_next(set, cursor);                                          \
    }                                                                          \
    size;                                                                      \
  })

#define set_transplant(set, dest_idx, src_idx)                                 \
  do {                                                                         \
    typeof(set.nodes) dest = set_get_node(set, dest_idx);                      \
    if (!set_is_valid_addr(dest->parent)) {                                    \
      set.root = src_idx;                                                      \
    } else {                                                                   \
      typeof(set.nodes) parent = set_get_node(set, dest->parent);              \
      if (dest_idx == parent->left) {                                          \
        parent->left = src_idx;                                                \
      } else {                                                                 \
        parent->right = src_idx;                                               \
      }                                                                        \
    }                                                                          \
    typeof(set.nodes) src = set_get_node(set, src_idx);                        \
    src->parent = dest->parent;                                                \
  } while (0)

#define set_type(entry_type)                                                   \
  struct {                                                                     \
    set_node *nodes;                                                           \
    entry_type *entries;                                                       \
    uint8_t *colors;                                                           \
    uint8_t *free_list;                                                        \
    uint8_t *inited;                                                           \
    size_t root;                                                               \
    size_t capacity;                                                           \
    uint64_t (*hash_fn)(entry_type);                                           \
  }

#define set_ult(set, f_direction)                                              \
  ({                                                                           \
    size_t cursor = set.root;                                                  \
    size_t retval = 0;                                                         \
    while (set_is_valid_addr(cursor) && set_is_inited(set, cursor)) {          \
      retval = cursor;                                                         \
      cursor = set_get_node(set, cursor)->f_direction;                         \
    }                                                                          \
    retval;                                                                    \
  })

#define set_ult_in_branch(set, node_idx, f_direction)                          \
  ({                                                                           \
    size_t idx = (node_idx);                                                   \
    typeof(set.nodes) node;                                                    \
    while (true) {                                                             \
      node = set_get_node(set, idx);                                           \
      if (!set_is_inited(set, node->f_direction)) {                            \
        break;                                                                 \
      }                                                                        \
      idx = node->f_direction;                                                 \
    }                                                                          \
    idx;                                                                       \
  })

#define set_write_bitval(set, addr, f_member, val)                             \
  do {                                                                         \
    size_t idx = set_idx(addr);                                                \
    size_t byte_idx = floor((float)idx / 8);                                   \
    size_t bit_idx = idx % 8;                                                  \
    uint8_t mask = 1 << (7 - bit_idx);                                         \
    if (val == 1) {                                                            \
      set.f_member[byte_idx] |= mask;                                          \
    } else {                                                                   \
      set.f_member[byte_idx] &= ~mask;                                         \
    }                                                                          \
  } while (0)

#define set_write_color(set, addr, val) set_write_bitval(set, addr, colors, val)

#define set_write_entry(set, addr, entry)                                      \
  do {                                                                         \
    size_t set_write_entry_idx = set_idx(addr);                                \
    assert(set.capacity > set_write_entry_idx);                                \
    set.entries[set_write_entry_idx] = entry;                                  \
    set_write_inited(set, addr, true);                                         \
  } while (0)

#define set_write_inited(set, addr, val)                                       \
  set_write_bitval(set, addr, inited, val)

#endif // !GENERIC_SET_H
