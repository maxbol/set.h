#ifndef GENERIC_SET_H
#define GENERIC_SET_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HASH_NIL 0
#define IDX_NIL 0
#define NODE_COLOR_BLACK 0
#define NODE_COLOR_RED 1

#define NODE_NIL                                                               \
  {                                                                            \
      .hash = HASH_NIL,                                                        \
      .left = IDX_NIL,                                                         \
      .right = IDX_NIL,                                                        \
  };

#define set_type(entry_type)                                                   \
  struct {                                                                     \
    struct node {                                                              \
      size_t left;                                                             \
      size_t right;                                                            \
      size_t parent;                                                           \
      uint64_t hash;                                                           \
      entry_type entry;                                                        \
      uint8_t color;                                                           \
    } *buf;                                                                    \
    size_t root;                                                               \
    size_t capacity;                                                           \
    uint64_t (*hash_fn)(entry_type);                                           \
    uint8_t *free_list;                                                        \
  }

#define SET_ALLOC_CHUNK 512

#define BIT_HEADER_ADDR ((size_t)1 << 63)
#define BIT_HEADER_IDX ((size_t)0xffffffffffffffff >> 1)

#define set_addr(idx) ((size_t)idx | BIT_HEADER_ADDR)
#define set_idx(addr) ((size_t)addr & BIT_HEADER_IDX)

#define set_get_node(set, addr)                                                \
  ({                                                                           \
    size_t a = (addr);                                                         \
    typeof(set.buf) retval = NULL;                                             \
    if ((a & BIT_HEADER_ADDR) != 0) {                                          \
      size_t idx = set_idx(a);                                                 \
      assert(set.capacity > idx);                                              \
      retval = &set.buf[idx];                                                  \
    }                                                                          \
    retval;                                                                    \
  })

#define set_get_parent(set, node) set_get_node(set, node->parent)

#define set_get_grandparent(set, node)                                         \
  set_get_parent(set, set_get_parent(set, node))

#define set_get_uncle(set, node, f_branch)                                     \
  set_get_node(set, set_get_grandparent(set, node)->f_branch)

#define set_get_sibling(set, node, f_branch)                                   \
  set_get_node(set, set_get_parent(set, node)->f_branch)

#define set_get_root(set) set_get_node(set, set.root)

#define set_init(set, hash_function)                                           \
  do {                                                                         \
    set.capacity = SET_ALLOC_CHUNK;                                            \
    set.buf = malloc(sizeof(typeof(*set.buf)) * set.capacity);                 \
    set.free_list = malloc(set.capacity / 8);                                  \
    memset(set.free_list, 0xff, set.capacity / 8);                             \
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
        set.buf[i] = (typeof(*set.buf))NODE_NIL;                               \
        retval = set_addr(i);                                                  \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if (!found_free) {                                                         \
      retval = set_addr(set.capacity);                                         \
      set.capacity *= 2;                                                       \
      set.buf = realloc(set.buf, sizeof(typeof(*set.buf)) * set.capacity);     \
      set.buf[retval] = (typeof(*set.buf))NODE_NIL;                            \
      set.free_list = realloc(set.free_list, set.capacity / 8);                \
      memset(&set.free_list[retval / 8], 0xff,                                 \
             (set.capacity / 8) - (retval / 8));                               \
      set.free_list[retval] = 0xff >> 1;                                       \
    }                                                                          \
    retval;                                                                    \
  })

#define set_free_node(set, addr)                                               \
  ({                                                                           \
    size_t free_list_byte_idx = floor(addr / 8);                               \
    size_t free_list_bit_idx = addr % 8;                                       \
    uint8_t bmask = 1 << (7 - free_list_bit_idx);                              \
    set.free_list[free_list_byte_idx] ^= bmask;                                \
  })

#define set_free(set)                                                          \
  do {                                                                         \
    free(set.buf);                                                             \
  } while (0)

#define set_seq_in_branch(set, node, f_branch, f_direction)                    \
  ({                                                                           \
    typeof(node) retval = NULL;                                                \
    if (node->hash != HASH_NIL) {                                              \
      retval = set_get_node(set, node->f_branch);                              \
                                                                               \
      if (retval->hash != HASH_NIL) {                                          \
        while (true) {                                                         \
          typeof(node) next = set_get_node(set, retval->f_direction);          \
          if (next->hash == HASH_NIL) {                                        \
            break;                                                             \
          }                                                                    \
          retval = next;                                                       \
        }                                                                      \
      } else {                                                                 \
        retval = NULL;                                                         \
      }                                                                        \
    }                                                                          \
                                                                               \
    retval;                                                                    \
  })

#define set_next_in_branch(set, node) set_seq_in_branch(set, node, right, left);
#define set_prev_in_branch(set, node) set_seq_in_branch(set, node, left, right);

#define set_seq(set, node, f_branch, f_direction)                              \
  ({                                                                           \
    typeof(node) retval = set_seq_in_branch(set, node, f_branch, f_direction); \
                                                                               \
    if (retval == NULL) {                                                      \
      uint64_t hash = (node)->hash;                                            \
      retval = set_get_parent(set, node);                                      \
      if (retval != NULL) {                                                    \
        do {                                                                   \
          if (set_get_node(set, retval->f_direction)->hash == hash) {          \
            break;                                                             \
          }                                                                    \
          hash = retval->hash;                                                 \
        } while ((retval = set_get_parent(set, retval)));                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    retval;                                                                    \
  })

#define set_next(set, node) set_seq(set, node, right, left)
#define set_prev(set, node) set_seq(set, node, left, right)

#define set_ult(set, f_branch, f_direction)                                    \
  ({                                                                           \
    typeof(set.buf) node = set_get_root(set);                                  \
    if (node != NULL) {                                                        \
      while (true) {                                                           \
        typeof(set.buf) next = set_seq(set, node, f_branch, f_direction);      \
        if (next == NULL) {                                                    \
          break;                                                               \
        }                                                                      \
        node = next;                                                           \
      }                                                                        \
    }                                                                          \
    node;                                                                      \
  })

#define set_first(set) set_ult(set, left, right)
#define set_last(set) set_ult(set, right, left)

#define find_node(set, key)                                                    \
  ({                                                                           \
    size_t n_idx = set.root;                                                   \
    while (set_get_node(set, n_idx)->hash != HASH_NIL) {                       \
      typeof(set.buf) rv = set_get_node(set, n_idx);                           \
      if (key > rv->hash) {                                                    \
        n_idx = rv->right;                                                     \
      } else if (key < rv->hash) {                                             \
        n_idx = rv->left;                                                      \
      } else {                                                                 \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    n_idx;                                                                     \
  })

// TODO(2025-02-03, Max Bolotin): Replace malloc with custom allocator
#define set_add(set, entry_var)                                                \
  do {                                                                         \
    uint64_t hash = set.hash_fn(entry_var);                                    \
    size_t leaf_idx = find_node(set, hash);                                    \
    typeof(set.buf) leaf = set_get_node(set, leaf_idx);                        \
                                                                               \
    if (leaf->hash != HASH_NIL) {                                              \
      break;                                                                   \
    }                                                                          \
                                                                               \
    size_t left_idx = set_alloc_new_node(set);                                 \
    size_t right_idx = set_alloc_new_node(set);                                \
    set_get_node(set, left_idx)->parent = leaf_idx;                            \
    set_get_node(set, right_idx)->parent = leaf_idx;                           \
                                                                               \
    *leaf = (typeof(*set.buf)){                                                \
        .color = NODE_COLOR_RED,                                               \
        .entry = entry_var,                                                    \
        .hash = hash,                                                          \
        .left = left_idx,                                                      \
        .parent = leaf->parent,                                                \
        .right = right_idx,                                                    \
    };                                                                         \
                                                                               \
    set_rb_insert_fixup(set, leaf_idx);                                        \
  } while (0)

#define set_rb_insert_fixup_dir(set, node_idx, f_branch, f_direction)          \
  do {                                                                         \
    typeof(set.buf) node = set_get_node(set, node_idx);                        \
    typeof(set.buf) uncle = set_get_uncle(set, node, f_branch);                \
    if (uncle->color == NODE_COLOR_RED) {                                      \
      set_get_parent(set, node)->color = NODE_COLOR_BLACK;                     \
      uncle->color = NODE_COLOR_BLACK;                                         \
      set_get_grandparent(set, node)->color = NODE_COLOR_RED;                  \
      node_idx = set_get_parent(set, node)->parent;                            \
    } else {                                                                   \
      if (node == set_get_sibling(set, node, f_branch)) {                      \
        node_idx = node->parent;                                               \
        node = set_get_node(set, node_idx);                                    \
        set_rot(set, node_idx, f_branch, f_direction);                         \
      }                                                                        \
      set_get_parent(set, node)->color = NODE_COLOR_BLACK;                     \
      set_get_grandparent(set, node)->color = NODE_COLOR_RED;                  \
      set_rot(set, set_get_parent(set, node)->parent, f_direction, f_branch);  \
    }                                                                          \
  } while (0)

#define set_rb_insert_fixup_left(set, node_idx)                                \
  set_rb_insert_fixup_dir(set, node_idx, right, left)
#define set_rb_insert_fixup_right(set, node_idx)                               \
  set_rb_insert_fixup_dir(set, node_idx, left, right)

#define set_rb_insert_fixup(set, node_idx)                                     \
  do {                                                                         \
    while (true) {                                                             \
      typeof(set.buf) node = set_get_node(set, node_idx);                      \
      typeof(set.buf) parent = set_get_parent(set, node);                      \
      if (parent == NULL || parent->color != NODE_COLOR_RED) {                 \
        break;                                                                 \
      }                                                                        \
      if (parent == set_get_sibling(set, parent, left)) {                      \
        set_rb_insert_fixup_left(set, node_idx);                               \
      } else {                                                                 \
        set_rb_insert_fixup_right(set, node_idx);                              \
      }                                                                        \
    }                                                                          \
    set_get_root(set)->color = NODE_COLOR_BLACK;                               \
  } while (0)

#define set_remove(set, entry)                                                 \
  do {                                                                         \
    uint64_t hash = set.hash_fn(entry);                                        \
    size_t node_idx = find_node(set, hash);                                    \
    typeof(set.buf) node = set_get_node(set, node_idx);                        \
                                                                               \
    if (node->hash != hash) {                                                  \
      break;                                                                   \
    }                                                                          \
                                                                               \
    size_t orphan_idx;                                                         \
    if (set_get_node(set, node->left)->hash == HASH_NIL) {                     \
      orphan_idx = set_get_node(set, node->right);                             \
    } else if (set_get_node(set, node->right)->hash == HASH_NIL) {             \
      orphan_idx = set_get_node(set, node->left);                              \
    } else {                                                                   \
      typeof(set.buf) next = set_next_in_branch(node);                         \
      assert(next != NULL);                                                    \
      node->hash = next->hash;                                                 \
      node->entry = next->entry;                                               \
      set_free_node(set, next->left);                                          \
      set_free_node(set, next->right);                                         \
      *next = (typeof(*set.root))NODE_NIL;                                     \
      break;                                                                   \
    }                                                                          \
                                                                               \
    typeof(set.buf) orphan = set_get_node(set, orphan_idx);                    \
    *node = *orphan;                                                           \
    set_free_node(orphan_idx);                                                 \
  } while (0)

#define set_getidx(set, v_idx)                                                 \
  ({                                                                           \
    typeof(set.buf) node = set_get_root(set);                                  \
    if (node->hash == HASH_NIL) {                                              \
      node = NULL;                                                             \
    } else {                                                                   \
      size_t idx = 0;                                                          \
      do {                                                                     \
        if (idx == v_idx) {                                                    \
          break;                                                               \
        }                                                                      \
        idx++;                                                                 \
      } while ((node = set_seq(node, right, left)));                           \
    }                                                                          \
    typeof(&node->entry) ref;                                                  \
    if (node == NULL) {                                                        \
      ref = NULL;                                                              \
    } else {                                                                   \
      ref = &node->entry;                                                      \
    }                                                                          \
    ref;                                                                       \
  })

#define set_size(set)                                                          \
  ({                                                                           \
    typeof(set.buf) node = set_first(set);                                     \
    size_t size = 0;                                                           \
    do {                                                                       \
      size++;                                                                  \
    } while ((node = set_next(set, node)));                                    \
    size;                                                                      \
  })

#define set_has(set, entry)                                                    \
  ({                                                                           \
    uint64_t hash = set.hash_fn(entry);                                        \
    size_t node_idx = find_node(set, hash);                                    \
    typeof(set.buf) node = set_get_node(set, node_idx);                        \
    node->hash == hash;                                                        \
  })

#define set_rot(set, node_idx, f_branch, f_direction)                          \
  do {                                                                         \
    typeof(set.buf) rot_node = set_get_node(set, node_idx);                    \
    size_t f_branch_idx = rot_node->f_branch;                                  \
    typeof(set.buf) f_branch = set_get_node(set, f_branch_idx);                \
    assert(f_branch->hash != HASH_NIL);                                        \
    rot_node->f_branch = f_branch->f_direction;                                \
    if (set_get_node(set, f_branch->f_direction)->hash != HASH_NIL) {          \
      set_get_node(set, f_branch->f_direction)->parent = node_idx;             \
    }                                                                          \
    f_branch->parent = rot_node->parent;                                       \
    if (set_get_parent(set, rot_node) == NULL) {                               \
      set.root = f_branch_idx;                                                 \
    } else if (rot_node == set_get_sibling(set, rot_node, f_direction)) {      \
      set_get_parent(set, rot_node)->f_direction = f_branch_idx;               \
    } else {                                                                   \
      set_get_parent(set, rot_node)->f_branch = f_branch_idx;                  \
    }                                                                          \
    f_branch->f_direction = node_idx;                                          \
    rot_node->parent = f_branch_idx;                                           \
  } while (0)

#define set_rot_left(set, node_idx) set_rot(set, node_idx, right, left)
#define set_rot_right(set, node_idx) set_rot(set, node_idx, left, right)

#endif // !GENERIC_SET_H
