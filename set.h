#ifndef GENERIC_SET_H
#define GENERIC_SET_H

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

#define SET_ALLOC_CHUNK 512

#define BIT_HEADER_ADDR ((size_t)1 << 63)
#define BIT_HEADER_IDX (~BIT_HEADER_ADDR)

// #define set_addr(idx) ((size_t)idx | BIT_HEADER_ADDR)
// #define set_idx(addr) ((size_t)addr & BIT_HEADER_IDX)
// #define set_is_valid_addr(addr) (((size_t)addr & BIT_HEADER_ADDR) != 0)
//
#define set_addr(idx) (idx) + 1
#define set_idx(addr) (addr) - 1
#define set_is_valid_addr(addr) ((size_t)addr != 0)

#define set_get_node(set, addr)                                                \
  ({                                                                           \
    size_t a = (addr);                                                         \
    typeof(set.nodes) retval = NULL;                                           \
    if (set_is_valid_addr(addr)) {                                             \
      size_t idx = set_idx(a);                                                 \
      assert(set.capacity > idx);                                              \
      retval = &set.nodes[idx];                                                \
    }                                                                          \
    retval;                                                                    \
  })

#define set_get_entry(set, addr)                                               \
  ({                                                                           \
    size_t idx = set_idx(addr);                                                \
    assert(set.capacity > idx);                                                \
    set.entries[idx];                                                          \
  })

#define set_write_entry(set, addr, entry)                                      \
  do {                                                                         \
    size_t set_write_entry_idx = set_idx(addr);                                \
    assert(set.capacity > set_write_entry_idx);                                \
    set.entries[set_write_entry_idx] = entry;                                  \
    set_write_inited(set, addr, true);                                         \
  } while (0)

#define set_clear_entry(set, addr)                                             \
  do {                                                                         \
    size_t clear_idx = set_idx(addr);                                          \
    assert(set.capacity > clear_idx);                                          \
    memset(&set.entries[clear_idx], 0x00, sizeof(typeof(set.entries)));        \
    set_write_inited(set, addr, false);                                        \
  } while (0)

#define set_get_sibling(set, node, f_branch)                                   \
  set_get_node(set, set_get_node(set, node->parent)->f_branch)

#define set_read_bitval(set, addr, f_member)                                   \
  ({                                                                           \
    size_t idx = set_idx(addr);                                                \
    size_t byte_idx = floor((float)idx / 8);                                   \
    size_t bit_idx = idx % 8;                                                  \
    uint8_t mask = 1 << (7 - bit_idx);                                         \
    (set.f_member[byte_idx] & mask) != 0;                                      \
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

#define set_read_color(set, addr) set_read_bitval(set, addr, colors)
#define set_write_color(set, addr, val) set_write_bitval(set, addr, colors, val)
#define set_read_inited(set, addr) set_read_bitval(set, addr, inited)
#define set_write_inited(set, addr, val)                                       \
  set_write_bitval(set, addr, inited, val)

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

#define set_free_node(set, addr)                                               \
  do {                                                                         \
    size_t free_list_byte_idx = floor((float)addr / 8);                        \
    size_t free_list_bit_idx = addr % 8;                                       \
    uint8_t bmask = 1 << (7 - free_list_bit_idx);                              \
    set.free_list[free_list_byte_idx] ^= bmask;                                \
  } while (0)

#define set_free(set)                                                          \
  do {                                                                         \
    free(set.nodes);                                                           \
    free(set.entries);                                                         \
    free(set.colors);                                                          \
    free(set.inited);                                                          \
    free(set.free_list);                                                       \
  } while (0)

#define set_seq_in_branch(set, node_idx, f_branch, f_direction)                \
  ({                                                                           \
    size_t idx = 0;                                                            \
    if (set_read_inited(set, node_idx) == 1) {                                 \
      typeof(set.nodes) node = set_get_node(set, node_idx);                    \
                                                                               \
      if (set_read_inited(set, node->f_branch) == 1) {                         \
        idx = node->f_branch;                                                  \
        while (true) {                                                         \
          node = set_get_node(set, idx);                                       \
          if (set_is_valid_addr(node->f_direction) &&                          \
              set_read_inited(set, node->f_direction) == 0) {                  \
            break;                                                             \
          }                                                                    \
          idx = node->f_direction;                                             \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    idx;                                                                       \
  })

#define set_next_in_branch(set, node_idx)                                      \
  set_seq_in_branch(set, node_idx, right, left);
#define set_prev_in_branch(set, node_idx)                                      \
  set_seq_in_branch(set, node_idx, left, right);

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

#define set_next(set, node_idx) set_seq(set, node_idx, right, left)
#define set_prev(set, node_idx) set_seq(set, node_idx, left, right)

#define set_ult(set, f_direction)                                              \
  ({                                                                           \
    size_t cursor = set.root;                                                  \
    size_t retval = 0;                                                         \
    while (set_is_valid_addr(cursor) && set_read_inited(set, cursor)) {        \
      retval = cursor;                                                         \
      cursor = set_get_node(set, cursor)->f_direction;                         \
    }                                                                          \
    retval;                                                                    \
  })

#define set_first(set) set_ult(set, left)
#define set_last(set) set_ult(set, right)

#define find_node(set, key)                                                    \
  ({                                                                           \
    size_t n_idx = set.root;                                                   \
    size_t find_node_retval;                                                   \
    do {                                                                       \
      find_node_retval = n_idx;                                                \
      typeof(set.nodes) node = set_get_node(set, n_idx);                       \
      if (key > node->hash) {                                                  \
        n_idx = node->right;                                                   \
      } else if (key < node->hash) {                                           \
        n_idx = node->left;                                                    \
      } else {                                                                 \
        break;                                                                 \
      }                                                                        \
    } while (set_is_valid_addr(n_idx));                                        \
    find_node_retval;                                                          \
  })

#define set_add(set, entry_var)                                                \
  do {                                                                         \
    uint64_t hash = set.hash_fn(entry_var);                                    \
    size_t leaf_idx = find_node(set, hash);                                    \
                                                                               \
    if (set_read_inited(set, leaf_idx) != 0) {                                 \
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
    if (set_read_color(set, uncle_idx) == NODE_COLOR_RED) {                    \
      set_write_color(set, parent_idx, NODE_COLOR_BLACK);                      \
      set_write_color(set, uncle_idx, NODE_COLOR_BLACK);                       \
      set_write_color(set, grandparent_idx, NODE_COLOR_RED);                   \
      node_idx = grandparent_idx;                                              \
    } else {                                                                   \
      if (node == set_get_sibling(set, node, f_branch)) {                      \
        node_idx = node->parent;                                               \
                                                                               \
        node = set_get_node(set, node_idx);                                    \
        parent_idx = node->parent;                                             \
                                                                               \
        parent = set_get_node(set, parent_idx);                                \
                                                                               \
        set_rot(set, node_idx, f_branch, f_direction);                         \
      }                                                                        \
      set_write_color(set, parent_idx, NODE_COLOR_BLACK);                      \
      if (parent->parent != 0) {                                               \
        set_write_color(set, parent->parent, NODE_COLOR_RED);                  \
        set_rot(set, parent->parent, f_direction, f_branch);                   \
      }                                                                        \
    }                                                                          \
  } while (0)

#define set_rb_insert_fixup_left(set, node_idx)                                \
  set_rb_insert_fixup_dir(set, node_idx, right, left)
#define set_rb_insert_fixup_right(set, node_idx)                               \
  set_rb_insert_fixup_dir(set, node_idx, left, right)

#define set_rb_insert_fixup(set, node_idx)                                     \
  do {                                                                         \
    while (true) {                                                             \
      typeof(set.nodes) node = set_get_node(set, node_idx);                    \
      typeof(set.nodes) parent = set_get_node(set, node->parent);              \
      if (parent == NULL ||                                                    \
          set_read_color(set, node->parent) != NODE_COLOR_RED) {               \
        break;                                                                 \
      }                                                                        \
      if (parent == set_get_sibling(set, parent, left)) {                      \
        set_rb_insert_fixup_left(set, node_idx);                               \
      } else {                                                                 \
        set_rb_insert_fixup_right(set, node_idx);                              \
      }                                                                        \
    }                                                                          \
    set_write_color(set, set.root, NODE_COLOR_BLACK);                          \
  } while (0)

#define set_remove(set, entry)                                                 \
  do {                                                                         \
    uint64_t hash = set.hash_fn(entry);                                        \
    size_t node_idx = find_node(set, hash);                                    \
    typeof(set.nodes) node = set_get_node(set, node_idx);                      \
                                                                               \
    if (node->hash != hash) {                                                  \
      break;                                                                   \
    }                                                                          \
                                                                               \
    size_t orphan_idx;                                                         \
    if (!set_read_inited(set, node->left)) {                                   \
      orphan_idx = node->right;                                                \
    } else if (!set_read_inited(set, node->right)) {                           \
      orphan_idx = node->left;                                                 \
    } else {                                                                   \
      size_t next_idx = set_next_in_branch(set, node_idx);                     \
      assert(set_read_inited(set, next_idx) == true);                          \
      typeof(set.nodes) next = set_get_node(set, next_idx);                    \
      bool next_orig_color = set_read_color(set, next_idx);                    \
      node->hash = next->hash;                                                 \
      set_write_entry(set, node_idx, set_get_entry(set, next_idx));            \
      set_write_color(set, node_idx, set_read_color(set, next_idx));           \
      set_free_node(set, next->left);                                          \
      set_free_node(set, next->right);                                         \
      *next = (typeof(*set.nodes))NODE_NIL;                                    \
      set_clear_entry(set, next_idx);                                          \
                                                                               \
      if (next_orig_color == NODE_COLOR_BLACK) {                               \
        set_delete_fixup(set, node_idx);                                       \
      }                                                                        \
      break;                                                                   \
    }                                                                          \
                                                                               \
    typeof(set.nodes) orphan = set_get_node(set, orphan_idx);                  \
    node->hash = orphan->hash;                                                 \
    node->left = orphan->left;                                                 \
    node->right = orphan->right;                                               \
    set_write_entry(set, node_idx, set_get_entry(set, orphan_idx));            \
    set_write_color(set, node_idx, set_read_color(set, orphan_idx));           \
    set_write_inited(set, node_idx, set_read_inited(set, orphan_idx));         \
                                                                               \
    set_free_node(set, orphan_idx);                                            \
  } while (0)

#define set_delete_fixup_dir(set, node_idx, f_branch, f_direction)             \
  do {                                                                         \
    typeof(set.nodes) node = set_get_node(set, node_idx);                      \
    typeof(set.nodes) parent = set_get_node(set, node->parent);                \
    size_t sibling_idx = parent->f_branch;                                     \
                                                                               \
    if (set_read_color(set, sibling_idx) == NODE_COLOR_RED) {                  \
      set_write_color(set, sibling_idx, NODE_COLOR_BLACK);                     \
      set_write_color(set, node->parent, NODE_COLOR_RED);                      \
      set_rot(set, node->parent, f_branch, f_direction);                       \
      sibling_idx = set_get_node(set, node->parent)->f_branch;                 \
    }                                                                          \
                                                                               \
    typeof(set.nodes) sibling = set_get_node(set, sibling_idx);                \
                                                                               \
    if (set_read_color(set, sibling->f_branch) == NODE_COLOR_BLACK &&          \
        set_read_color(set, sibling->f_direction) == NODE_COLOR_BLACK) {       \
      set_write_color(set, sibling_idx, NODE_COLOR_RED);                       \
      node_idx = node->parent;                                                 \
      node = set_get_node(set, node_idx);                                      \
    } else {                                                                   \
      if (set_read_color(set, sibling->f_branch) == NODE_COLOR_BLACK) {        \
        set_write_color(set, sibling->f_direction, NODE_COLOR_BLACK);          \
        set_write_color(set, sibling_idx, NODE_COLOR_RED);                     \
        set_rot(set, sibling_idx, f_direction, f_branch);                      \
        sibling_idx =                                                          \
            set_get_node(set, set_get_node(set, node_idx)->parent)->f_branch;  \
        sibling = set_get_node(set, sibling_idx);                              \
      }                                                                        \
      set_write_color(set, sibling_idx, set_read_color(set, node->parent));    \
      set_write_color(set, node->parent, NODE_COLOR_BLACK);                    \
      set_write_color(set, sibling->f_branch, NODE_COLOR_BLACK);               \
      set_rot(set, node->parent, f_branch, f_direction);                       \
      node_idx = set.root;                                                     \
    }                                                                          \
  } while (0)

#define set_delete_fixup(set, node_idx)                                        \
  do {                                                                         \
    while (node_idx != set.root) {                                             \
      typeof(set.nodes) node = set_get_node(set, node_idx);                    \
      bool color = set_read_color(set, node_idx);                              \
      if (color != NODE_COLOR_BLACK) {                                         \
        break;                                                                 \
      }                                                                        \
      if (node == set_get_sibling(set, node, left)) {                          \
        set_delete_fixup_dir(set, node_idx, right, left);                      \
      } else {                                                                 \
        set_delete_fixup_dir(set, node_idx, right, left);                      \
      }                                                                        \
    }                                                                          \
    set_write_color(set, node_idx, NODE_COLOR_BLACK);                          \
  } while (0)

#define set_size(set)                                                          \
  ({                                                                           \
    size_t cursor = set_first(set);                                            \
    size_t size = 0;                                                           \
    while (set_is_valid_addr(cursor) && set_read_inited(set, cursor)) {        \
      size++;                                                                  \
      cursor = set_next(set, cursor);                                          \
    }                                                                          \
    size;                                                                      \
  })

#define set_has(set, entry)                                                    \
  ({                                                                           \
    uint64_t hash = set.hash_fn(entry);                                        \
    size_t node_idx = find_node(set, hash);                                    \
    typeof(set.nodes) node = set_get_node(set, node_idx);                      \
    set_read_inited(set, node_idx) && node->hash == hash;                      \
  })

#define set_rot(set, node_idx, f_branch, f_direction)                          \
  do {                                                                         \
    size_t n_idx = (node_idx);                                                 \
    typeof(set.nodes) rot_node = set_get_node(set, n_idx);                     \
    size_t f_branch_idx = rot_node->f_branch;                                  \
    assert(set_is_valid_addr(f_branch_idx));                                   \
    typeof(set.nodes) f_branch = set_get_node(set, f_branch_idx);              \
    rot_node->f_branch = f_branch->f_direction;                                \
    if (set_read_inited(set, f_branch->f_direction)) {                         \
      set_get_node(set, f_branch->f_direction)->parent = n_idx;                \
    }                                                                          \
    f_branch->parent = rot_node->parent;                                       \
    if (set_get_node(set, rot_node->parent) == NULL) {                         \
      set.root = f_branch_idx;                                                 \
    } else if (rot_node == set_get_sibling(set, rot_node, f_direction)) {      \
      set_get_node(set, rot_node->parent)->f_direction = f_branch_idx;         \
    } else {                                                                   \
      set_get_node(set, rot_node->parent)->f_branch = f_branch_idx;            \
    }                                                                          \
    f_branch->f_direction = n_idx;                                             \
    rot_node->parent = f_branch_idx;                                           \
  } while (0)

#define set_transplant(set, node_idx, target_idx)                              \
  do {                                                                         \
    typeof(set.node) node = set_get_node(set, node_idx);                       \
    if (!set_is_valid_addr(node->parent)) {                                    \
      set.root = target_idx;                                                   \
    } else {                                                                   \
      typeof(set.node) parent = set_get_node(set, node->parent);               \
      if (node_idx = parent->left) {                                           \
        parent->left = target_idx;                                             \
      } else {                                                                 \
        parent->right = target_idx;                                            \
      }                                                                        \
    }                                                                          \
    typeof(set.node) target = set_get_node(set, target_idx);                   \
    target->parent = node->parent;                                             \
  } while (0)

#define set_rot_left(set, node_idx) set_rot(set, node_idx, right, left)
#define set_rot_right(set, node_idx) set_rot(set, node_idx, left, right)

#endif // !GENERIC_SET_H
