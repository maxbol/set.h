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
#define lush_trace()
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
#define FREE_LIST_STOP 0

#define NODE_NIL                                                               \
  {                                                                            \
      .hash = HASH_NIL,                                                        \
      .left = IDX_NIL,                                                         \
      .right = IDX_NIL,                                                        \
  };

#define COLLISION_NIL                                                          \
  { .next = 0, .prev = 0 }

#define TREE_SPECIAL_IDX 1
#define TREE_SIZE_LIMIT (UINT32_MAX - TREE_SPECIAL_IDX)

typedef uint32_t tree_addr_t;
typedef uint32_t tree_idx_t;

typedef struct {
  tree_addr_t left;
  tree_addr_t right;
  tree_addr_t parent;
  uint64_t hash;
} tree_node_t;

typedef struct {
  tree_addr_t next;
  tree_addr_t prev;
} tree_collision_t;

#define ALLOC_CHUNK 512

#define map_add(map, key_var, value_var)                                       \
  do {                                                                         \
    tree_addr_t leaf_addr = tree_add(map, key_var, map_alloc_new_node,         \
                                     map_write_key, map_find_duplicate);       \
                                                                               \
    if (tree_is_valid_addr(leaf_addr)) {                                       \
      map_write_value(map, leaf_addr, value_var);                              \
    }                                                                          \
  } while (0)

#define map_alloc_new_node(map)                                                \
  tree_alloc_new_node(map, map_create_entry, map_realloc_entries)

#define map_clear_entry(map, addr)                                             \
  do {                                                                         \
    tree_idx_t clear_idx = tree_idx(addr);                                     \
    assert(map.capacity > clear_idx);                                          \
    memset(&map.keys[clear_idx], 0x00, sizeof(typeof(map.keys)));              \
    memset(&map.values[clear_idx], 0x00, sizeof(typeof(map.values)));          \
  } while (0)

#define map_clone(map)                                                         \
  tree_clone(set, map_malloc_entries, map_get_key, map_write_key)

#define map_create_entry(map, idx)                                             \
  do {                                                                         \
    memset(&map.keys[idx], 0x00, sizeof(typeof(*map.keys)));                   \
    memset(&map.values[idx], 0x00, sizeof(typeof(*map.values)));               \
  } while (0)

#define map_empty(map) tree_empty(map, map_init, map_free)

#define map_find_duplicate(map, node_addr, key_var)                            \
  tree_find_duplicate(map, node_addr, key_var, map_get_key, keys)

#define map_find_node_entry(map, hash_value, key_var)                          \
  tree_find_node_entry(map, hash_value, key_var, map_find_duplicate)

#define map_free(map) tree_free(map, map_free_data)

#define map_free_data(set)                                                     \
  do {                                                                         \
    free(set.keys);                                                            \
    free(set.values);                                                          \
  } while (0)

#define map_get(map, key)                                                      \
  ({                                                                           \
    uint64_t hash = map.hash_fn(key);                                          \
    tree_addr_t node_addr = map_find_node_entry(map, hash, key);               \
    typeof(map.values) retval = NULL;                                          \
    if (tree_is_valid_addr(node_addr)) {                                       \
      tree_idx_t idx = tree_idx(node_addr);                                    \
      assert(map.capacity > idx);                                              \
      retval = &map.values[idx];                                               \
    }                                                                          \
    retval;                                                                    \
  })

#define map_get_key(map, addr)                                                 \
  ({                                                                           \
    tree_idx_t idx = tree_idx(addr);                                           \
    assert(map.capacity > idx);                                                \
    map.keys[idx];                                                             \
  })

#define map_get_value(map, addr)                                               \
  ({                                                                           \
    tree_idx_t idx = tree_idx(addr);                                           \
    assert(map.capacity > idx);                                                \
    map.values[idx];                                                           \
  })

#define map_has(map, key)                                                      \
  ({                                                                           \
    uint64_t hash = map.hash_fn(key);                                          \
    tree_addr_t node_addr = map_find_node_entry(map, hash, key);               \
    tree_is_valid_addr(node_addr);                                             \
  })

#define map_init(set, hash_function, equals_function)                          \
  tree_init(map, hash_function, equals_function, map_malloc_entries,           \
            map_alloc_new_node)

#define map_malloc_entries(map)                                                \
  do {                                                                         \
    map.keys = malloc(sizeof(typeof(*map.keys)) * map.capacity);               \
    map.values = malloc(sizeof(typeof(*map.values)) * map.capacity);           \
  } while (0)

#define map_realloc_entries(map)                                               \
  do {                                                                         \
    map.keys = realloc(map.keys, sizeof(typeof(*map.keys)) * map.capacity);    \
    map.values =                                                               \
        realloc(map.keys, sizeof(typeof(*map.values)) * map.capacity);         \
  } while (0)

#define map_remove(set, entry)                                                 \
  tree_remove(set, entry, map_find_node_entry, map_clear_entry)

#define map_size(tree) tree_size(tree)

#define map_type(key_type, value_type)                                         \
  struct {                                                                     \
    key_type *keys;                                                            \
    value_type *values;                                                        \
    uint64_t (*hash_fn)(key_type);                                             \
    bool (*equals_fn)(key_type, key_type);                                     \
    tree_type_fields()                                                         \
  }

#define map_write_key(map, addr, key)                                          \
  do {                                                                         \
    tree_idx_t map_write_key_idx = tree_idx(addr);                             \
    assert(map.capacity > map_write_key_idx);                                  \
    map.keys[map_write_key_idx] = key;                                         \
  } while (0)

#define map_write_value(map, addr, value)                                      \
  do {                                                                         \
    tree_idx_t map_write_value_idx = tree_idx(addr);                           \
    assert(map.capacity > map_write_value_idx);                                \
    map.values[map_write_value_idx] = value;                                   \
  } while (0)

#define set_add(set, entry_var)                                                \
  tree_add(set, entry_var, set_alloc_new_node, set_write_entry,                \
           set_find_duplicate)

#define set_alloc_new_node(set)                                                \
  tree_alloc_new_node(set, set_create_entry, set_realloc_entries)

#define set_clear_entry(set, addr)                                             \
  do {                                                                         \
    tree_idx_t clear_idx = tree_idx(addr);                                     \
    assert(set.capacity > clear_idx);                                          \
    memset(&set.entries[clear_idx], 0x00, sizeof(typeof(set.entries)));        \
    tree_write_inited(set, addr, false);                                       \
  } while (0)

#define set_clone(set)                                                         \
  tree_clone(set, set_malloc_entries, set_get_entry, set_write_entry)

#define set_create_entry(set, idx)                                             \
  memset(&set.entries[idx], 0x00, sizeof(typeof(*set.entries)))

#define set_empty(set) tree_empty(set, set_init, set_free)

#define set_find_duplicate(set, node_addr, entry_var)                          \
  tree_find_duplicate(set, node_addr, entry_var, set_get_entry, entries)

#define set_find_node_entry(set, hash_value, entry_var)                        \
  tree_find_node_entry(set, hash_value, entry_var, set_find_duplicate)

#define set_free(set) tree_free(set, set_free_data)

#define set_free_data(set)                                                     \
  do {                                                                         \
    free(set.entries);                                                         \
  } while (0)

#define set_get_entry(set, addr)                                               \
  ({                                                                           \
    tree_idx_t idx = tree_idx(addr);                                           \
    assert(set.capacity > idx);                                                \
    set.entries[idx];                                                          \
  })

#define set_has(set, entry)                                                    \
  ({                                                                           \
    uint64_t hash = set.hash_fn(entry);                                        \
    tree_addr_t node_addr = set_find_node_entry(set, hash, entry);             \
    tree_is_valid_addr(node_addr);                                             \
  })

#define set_init(set, hash_function, equals_function)                          \
  tree_init(set, hash_function, equals_function, set_malloc_entries,           \
            set_alloc_new_node)
#define set_malloc_entries(set)                                                \
  set.entries = malloc(sizeof(typeof(*set.entries)) * set.capacity)

#define set_realloc_entries(set)                                               \
  set.entries =                                                                \
      realloc(set.entries, sizeof(typeof(*set.entries)) * set.capacity)

#define set_remove(set, entry)                                                 \
  tree_remove(set, entry, set_find_node_entry, set_clear_entry)

#define set_size(tree) tree_size(tree)

#define set_type(entry_type)                                                   \
  struct {                                                                     \
    entry_type *entries;                                                       \
    uint64_t (*hash_fn)(entry_type);                                           \
    bool (*equals_fn)(entry_type, entry_type);                                 \
    tree_type_fields()                                                         \
  }

#define set_write_entry(set, addr, entry)                                      \
  do {                                                                         \
    tree_idx_t set_write_entry_idx = tree_idx(addr);                           \
    assert(set.capacity > set_write_entry_idx);                                \
    set.entries[set_write_entry_idx] = entry;                                  \
  } while (0)

#define tree_add(tree, entry_var, alloc_new_node, tree_write_entry,            \
                 find_duplicate)                                               \
  ({                                                                           \
    tree_addr_t retval = 0;                                                    \
    do {                                                                       \
      uint64_t hash = tree.hash_fn(entry_var);                                 \
      start_trace(1, hash, trace_span("Adding entry"));                        \
      tree_addr_t leaf_addr = tree_find_node(tree, hash);                      \
                                                                               \
      if (tree_is_inited(tree, leaf_addr) != 0) {                              \
        start_trace(18, hash, trace_span("Handle deduplication"));             \
        tree_addr_t duplicate_addr =                                           \
            find_duplicate(tree, leaf_addr, entry_var);                        \
                                                                               \
        if (tree_is_valid_addr(duplicate_addr)) {                              \
          retval = 0;                                                          \
          trace(trace_info("Entry already exists in tree"));                   \
          end_trace();                                                         \
          end_trace();                                                         \
          break;                                                               \
        }                                                                      \
                                                                               \
        tree_collision_t *collision = tree_get_collision(tree, leaf_addr);     \
                                                                               \
        while (tree_is_valid_addr(collision->next)) {                          \
          leaf_addr = collision->next;                                         \
          collision = tree_get_collision(tree, leaf_addr);                     \
        }                                                                      \
        tree_addr_t next;                                                      \
        tree_addr_t n = tree_get_node(tree, leaf_addr)->right;                 \
        while (tree_is_valid_addr(n)) {                                        \
          next = n;                                                            \
          n = tree_get_node(tree, next)->left;                                 \
        }                                                                      \
        collision->next = next;                                                \
        tree_get_collision(tree, next)->prev = leaf_addr;                      \
        leaf_addr = next;                                                      \
        end_trace();                                                           \
      }                                                                        \
                                                                               \
      start_trace(19, hash, trace_span("Allocing new leaf nodes"));            \
      tree_addr_t left_addr = alloc_new_node(tree);                            \
      tree_addr_t right_addr = alloc_new_node(tree);                           \
      end_trace();                                                             \
      tree_get_node(tree, left_addr)->parent = leaf_addr;                      \
      tree_get_node(tree, right_addr)->parent = leaf_addr;                     \
      tree_write_color(tree, left_addr, NODE_COLOR_BLACK);                     \
      tree_write_color(tree, right_addr, NODE_COLOR_BLACK);                    \
                                                                               \
      tree_node_t *leaf = tree_get_node(tree, leaf_addr);                      \
                                                                               \
      *leaf = (tree_node_t){                                                   \
          .hash = hash,                                                        \
          .left = left_addr,                                                   \
          .parent = leaf->parent,                                              \
          .right = right_addr,                                                 \
      };                                                                       \
                                                                               \
      tree_write_entry(tree, leaf_addr, entry_var);                            \
      tree_write_inited(tree, leaf_addr, true);                                \
      tree_write_color(tree, leaf_addr, NODE_COLOR_RED);                       \
                                                                               \
      tree_rb_insert_fixup(tree, leaf_addr);                                   \
      end_trace();                                                             \
                                                                               \
      retval = leaf_addr;                                                      \
                                                                               \
    } while (0);                                                               \
    retval;                                                                    \
  })

#define tree_addr(idx) (idx) + TREE_SPECIAL_IDX

#define tree_alloc_new_node(set, create_entry, realloc_entries)                \
  ({                                                                           \
    tree_addr_t retval = set.free_list_start;                                  \
    if (tree_is_valid_addr(retval)) {                                          \
      start_trace(20, retval, trace_span("Using existing free slot\n"));       \
      tree_idx_t i = tree_idx(retval);                                         \
      tree_idx_t byte_idx = floor((float)i / 8);                               \
      uint8_t bit_idx = i % 8;                                                 \
      uint8_t bmask = 1 << bit_idx;                                            \
      set.inited[byte_idx] &= ~bmask;                                          \
      set.colors[byte_idx] &= ~bmask;                                          \
      set.nodes[i] = (tree_node_t)NODE_NIL;                                    \
      set.collisions[i] = (tree_collision_t)COLLISION_NIL;                     \
      if (i >= set.count) {                                                    \
        if (i + 1 >= set.capacity) {                                           \
          set.free_list_start = FREE_LIST_STOP;                                \
        } else {                                                               \
          set.free_list_start = retval + 1;                                    \
        }                                                                      \
      } else {                                                                 \
        set.free_list_start = set.free_list[i];                                \
      }                                                                        \
      set.free_list[i] = FREE_LIST_STOP;                                       \
      create_entry(set, i);                                                    \
      end_trace();                                                             \
    } else {                                                                   \
      start_trace(21, 0, trace_span("Reallocating data"));                     \
      tree_idx_t i = set.capacity;                                             \
      retval = tree_addr(i);                                                   \
      set.capacity *= 2;                                                       \
      tree_idx_t flag_cap = set.capacity / 8;                                  \
      tree_idx_t flag_i = i / 8;                                               \
      tree_idx_t flag_offset = flag_cap - flag_i;                              \
                                                                               \
      start_trace(23, 0, trace_span("Realloc sys calls"));                     \
      set.nodes = realloc(set.nodes, sizeof(tree_node_t) * set.capacity);      \
      set.collisions =                                                         \
          realloc(set.collisions, sizeof(tree_collision_t) * set.capacity);    \
      set.free_list =                                                          \
          realloc(set.free_list, sizeof(tree_addr_t) * set.capacity);          \
      set.colors = realloc(set.colors, flag_cap);                              \
      set.inited = realloc(set.inited, flag_cap);                              \
      end_trace();                                                             \
                                                                               \
      start_trace(22, 0, trace_span("Free list expansion"));                   \
      set.free_list_start = tree_addr(i + 1);                                  \
      set.free_list[i] = FREE_LIST_STOP;                                       \
      set.free_list[set.capacity - 1] = FREE_LIST_STOP;                        \
      end_trace();                                                             \
                                                                               \
      realloc_entries(set);                                                    \
                                                                               \
      memset(&set.colors[flag_i], 0x00, flag_offset);                          \
      memset(&set.inited[flag_i], 0x00, flag_offset);                          \
                                                                               \
      set.nodes[i] = (tree_node_t)NODE_NIL;                                    \
      set.collisions[i] = (tree_collision_t)COLLISION_NIL;                     \
                                                                               \
      create_entry(set, i);                                                    \
                                                                               \
      end_trace();                                                             \
    }                                                                          \
    set.count++;                                                               \
    retval;                                                                    \
  })

#define tree_clone(tree, malloc_entries, get_entry, write_entry)               \
  ({                                                                           \
    typeof(tree) clone;                                                        \
    typeof(tree.hash_fn) hash_function = tree.hash_fn;                         \
    typeof(tree.equals_fn) equals_function = tree.equals_fn;                   \
                                                                               \
    clone.capacity = tree.capacity;                                            \
    tree_idx_t flag_cap = clone.capacity / 8;                                  \
                                                                               \
    clone.root = tree.root;                                                    \
    clone.nodes = malloc(sizeof(tree_node_t) * tree.capacity);                 \
    clone.collisions = malloc(sizeof(tree_collision_t) * tree.capacity);       \
    clone.entries = malloc(sizeof(typeof(*clone.entries)) * tree.capacity);    \
    clone.free_list = malloc(sizeof(tree_addr_t) * tree.capacity);             \
    clone.colors = malloc(flag_cap);                                           \
    clone.inited = malloc(flag_cap);                                           \
    malloc_entries(clone);                                                     \
                                                                               \
    memset(clone.free_list, 0x00, sizeof(tree_addr_t) * tree.capacity);        \
    memset(clone.colors, 0x00, flag_cap);                                      \
    memset(clone.inited, 0x00, flag_cap);                                      \
                                                                               \
    clone.free_list_start = tree.free_list_start;                              \
                                                                               \
    for (tree_idx_t i = 0; i < tree.capacity; i++) {                           \
      tree_addr_t a = tree_addr(i);                                            \
      clone.free_list[i] = tree.free_list[i];                                  \
      clone.collisions[i] = tree.collisions[i];                                \
      clone.nodes[i] = tree.nodes[i];                                          \
      write_entry(clone, a, get_entry(tree, a));                               \
      tree_write_color(clone, a, tree_is_red(tree, a));                        \
      tree_write_inited(clone, a, tree_is_inited(tree, a));                    \
    }                                                                          \
                                                                               \
    clone.hash_fn = hash_function;                                             \
    clone.equals_fn = equals_function;                                         \
                                                                               \
    clone;                                                                     \
  })

#define tree_delete_fixup(tree, node_addr)                                     \
  do {                                                                         \
    while (node_addr != tree.root) {                                           \
      tree_node_t *node = tree_get_node(tree, node_addr);                      \
      start_trace(2, node->hash, trace_span("Fixing up %lld"), node->hash);    \
      bool color = tree_is_red(tree, node_addr);                               \
      if (color != NODE_COLOR_BLACK) {                                         \
        trace(trace_info("Node is red, stop traversing"));                     \
        end_trace();                                                           \
        break;                                                                 \
      }                                                                        \
      if (node == tree_get_sibling(tree, node, left)) {                        \
        trace(trace_info("Node is in parents left subtree"));                  \
        tree_delete_fixup_dir(tree, node_addr, right, left);                   \
      } else {                                                                 \
        tree_delete_fixup_dir(tree, node_addr, left, right);                   \
      }                                                                        \
      end_trace();                                                             \
    }                                                                          \
    tree_write_color(tree, node_addr, NODE_COLOR_BLACK);                       \
  } while (0)

#define tree_delete_fixup_dir(tree, node_addr, f_branch, f_direction)          \
  do {                                                                         \
    tree_node_t *node = tree_get_node(tree, node_addr);                        \
    tree_node_t *parent = tree_get_node(tree, node->parent);                   \
    tree_addr_t sibling_addr = parent->f_branch;                               \
                                                                               \
    if (tree_is_red(tree, sibling_addr) == NODE_COLOR_RED) {                   \
      tree_write_color(tree, sibling_addr, NODE_COLOR_BLACK);                  \
      tree_write_color(tree, node->parent, NODE_COLOR_RED);                    \
      tree_rot(tree, node->parent, f_branch, f_direction);                     \
      sibling_addr = tree_get_node(tree, node->parent)->f_branch;              \
    }                                                                          \
                                                                               \
    tree_node_t *sibling = tree_get_node(tree, sibling_addr);                  \
                                                                               \
    if (tree_is_red(tree, sibling->f_branch) == NODE_COLOR_BLACK &&            \
        tree_is_red(tree, sibling->f_direction) == NODE_COLOR_BLACK) {         \
      tree_write_color(tree, sibling_addr, NODE_COLOR_RED);                    \
      node_addr = node->parent;                                                \
      node = tree_get_node(tree, node_addr);                                   \
    } else {                                                                   \
      if (tree_is_red(tree, sibling->f_branch) == NODE_COLOR_BLACK) {          \
        tree_write_color(tree, sibling->f_direction, NODE_COLOR_BLACK);        \
        tree_write_color(tree, sibling_addr, NODE_COLOR_RED);                  \
        tree_rot(tree, sibling_addr, f_direction, f_branch);                   \
        sibling_addr =                                                         \
            tree_get_node(tree, tree_get_node(tree, node_addr)->parent)        \
                ->f_branch;                                                    \
        sibling = tree_get_node(tree, sibling_addr);                           \
      }                                                                        \
      tree_write_color(tree, sibling_addr, tree_is_red(tree, node->parent));   \
      tree_write_color(tree, node->parent, NODE_COLOR_BLACK);                  \
      tree_write_color(tree, sibling->f_branch, NODE_COLOR_BLACK);             \
      tree_rot(tree, node->parent, f_branch, f_direction);                     \
      node_addr = tree.root;                                                   \
    }                                                                          \
  } while (0)

#define tree_empty(tree, set_init, set_free)                                   \
  do {                                                                         \
    typeof(tree.hash_fn) hash_fn = tree.hash_fn;                               \
    typeof(tree.equals_fn) equals_fn = tree.equals_fn;                         \
    set_free(tree);                                                            \
    set_init(tree, hash_fn, equals_fn);                                        \
  } while (0)

#define tree_find_duplicate(tree, node_addr, entry_var, tree_get_entry,        \
                            f_entries)                                         \
  ({                                                                           \
    tree_addr_t retval = 0;                                                    \
    typeof(*tree.f_entries) entry_val = (entry_var);                           \
                                                                               \
    for (int i = 0; i < 2; i++) {                                              \
      tree_addr_t next = node_addr;                                            \
                                                                               \
      do {                                                                     \
        typeof(*tree.f_entries) entry = tree_get_entry(tree, next);            \
        if ((next != node_addr || i == 0) &&                                   \
            tree.equals_fn(entry, entry_val)) {                                \
          retval = next;                                                       \
          break;                                                               \
        }                                                                      \
        tree_collision_t *collision = tree_get_collision(tree, next);          \
        next = i == 1 ? collision->next : collision->prev;                     \
      } while (tree_is_valid_addr(next));                                      \
                                                                               \
      if (tree_is_valid_addr(retval)) {                                        \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    retval;                                                                    \
  })

#define tree_find_node(tree, hash_value)                                       \
  ({                                                                           \
    tree_addr_t n_addr = tree.root;                                            \
    tree_addr_t find_node_retval;                                              \
    while (tree_is_valid_addr(n_addr)) {                                       \
      find_node_retval = n_addr;                                               \
      tree_node_t *node = tree_get_node(tree, n_addr);                         \
      if ((hash_value) > node->hash) {                                         \
        n_addr = node->right;                                                  \
      } else if ((hash_value) < node->hash) {                                  \
        n_addr = node->left;                                                   \
      } else {                                                                 \
        break;                                                                 \
      }                                                                        \
    };                                                                         \
    find_node_retval;                                                          \
  })

#define tree_find_node_entry(tree, hash_value, entry_var, tree_find_duplicate) \
  ({                                                                           \
    uint64_t hash_val = (hash_value);                                          \
    tree_addr_t node_addr = tree_find_node(tree, hash_val);                    \
    tree_addr_t retval = 0;                                                    \
    if (tree_is_inited(tree, node_addr)) {                                     \
      retval = tree_find_duplicate(tree, node_addr, entry_var);                \
    }                                                                          \
    retval;                                                                    \
  })

#define tree_first(tree) tree_ult(tree, left)

#define tree_free(tree, free_data)                                             \
  do {                                                                         \
    free(tree.nodes);                                                          \
    free(tree.colors);                                                         \
    free(tree.inited);                                                         \
    free(tree.free_list);                                                      \
    free_data(tree);                                                           \
  } while (0)

#define tree_free_node(tree, addr)                                             \
  do {                                                                         \
    tree_idx_t idx = tree_idx(addr);                                           \
    tree.free_list[idx] = tree.free_list_start;                                \
    tree.free_list_start = addr;                                               \
  } while (0)

#define tree_get_collision(tree, addr)                                         \
  ({                                                                           \
    tree_idx_t idx = tree_idx(addr);                                           \
    if (tree.capacity <= idx) {                                                \
      printf(                                                                  \
          "Warning: tree capacity (%zu) is less than or equal to index %d\n",  \
          tree.capacity, idx);                                                 \
    }                                                                          \
    assert(tree.capacity > idx);                                               \
    &tree.collisions[idx];                                                     \
  })

#define tree_get_node(tree, addr)                                              \
  ({                                                                           \
    tree_addr_t a = (addr);                                                    \
    tree_node_t *retval = NULL;                                                \
    if (tree_is_valid_addr(a)) {                                               \
      tree_idx_t idx = tree_idx(a);                                            \
      assert(tree.capacity > idx);                                             \
      retval = &tree.nodes[idx];                                               \
    }                                                                          \
    retval;                                                                    \
  })

#define tree_get_sibling(tree, node, f_branch)                                 \
  tree_get_node(tree, tree_get_node(tree, node->parent)->f_branch)

#define tree_idx(addr) (addr) - TREE_SPECIAL_IDX

#define tree_init(tree, hash_function, equals_function, malloc_entries,        \
                  alloc_new_node)                                              \
  do {                                                                         \
    tree.capacity = ALLOC_CHUNK;                                               \
    tree.count = 0;                                                            \
    tree.nodes = malloc(sizeof(tree_node_t) * tree.capacity);                  \
    tree.collisions = malloc(sizeof(tree_collision_t) * tree.capacity);        \
    tree.free_list = malloc(sizeof(tree_addr_t) * tree.capacity);              \
    tree.colors = malloc(tree.capacity / 8);                                   \
    tree.inited = malloc(tree.capacity / 8);                                   \
    tree.free_list_start = tree_addr(0);                                       \
    memset(tree.free_list, 0x00, sizeof(tree_addr_t) * tree.capacity);         \
    tree.free_list[tree.capacity - 1] = FREE_LIST_STOP;                        \
    malloc_entries(tree);                                                      \
    memset(tree.colors, 0x00, tree.capacity / 8);                              \
    memset(tree.inited, 0x00, tree.capacity / 8);                              \
    tree.root = alloc_new_node(tree);                                          \
    tree.hash_fn = hash_function;                                              \
    tree.equals_fn = equals_function;                                          \
  } while (0)

#define tree_is_inited(tree, addr) tree_read_bitval(tree, addr, inited)
#define tree_is_red(tree, addr) tree_read_bitval(tree, addr, colors)
#define tree_is_valid_addr(addr) ((tree_addr_t)addr >= TREE_SPECIAL_IDX)

#define tree_last(tree) tree_ult(tree, right)

#define tree_max_in_branch(tree, node_addr)                                    \
  tree_ult_in_branch(tree, node_addr, right);
#define tree_min_in_branch(tree, node_addr)                                    \
  tree_ult_in_branch(tree, node_addr, left);

#define tree_next(tree, node_addr) tree_seq(tree, node_addr, right, left)
#define tree_next_in_branch(tree, node_addr)                                   \
  tree_seq_in_branch(tree, node_addr, right, left);

#define tree_prev(tree, node_addr) tree_seq(tree, node_addr, left, right)
#define tree_prev_in_branch(tree, node_addr)                                   \
  tree_seq_in_branch(tree, node_addr, left, right);

#define tree_rb_insert_fixup(tree, node_addr)                                  \
  do {                                                                         \
    tree_addr_t addr = (node_addr);                                            \
    start_trace(17, tree_get_node(tree, addr)->hash,                           \
                trace_span("Running insert fixup\n"));                         \
    while (true) {                                                             \
      tree_node_t *node = tree_get_node(tree, addr);                           \
      start_trace(3, node->hash, trace_span("Fixing up node %lld"),            \
                  node->hash);                                                 \
      tree_node_t *parent = tree_get_node(tree, node->parent);                 \
      if (parent == NULL ||                                                    \
          tree_is_red(tree, node->parent) != NODE_COLOR_RED) {                 \
        trace(trace_info("Parent is NULL or black, doing nothing"));           \
        end_trace();                                                           \
        break;                                                                 \
      }                                                                        \
      if (parent == tree_get_sibling(tree, parent, left)) {                    \
        trace(trace_info(                                                      \
            "Parent is in its parents left tree, doing fixup_left"));          \
        tree_rb_insert_fixup_left(tree, addr);                                 \
      } else {                                                                 \
        trace(trace_info("Parent is in its parents right tree, "               \
                         "doing fixup_right"));                                \
        tree_rb_insert_fixup_right(tree, addr);                                \
      }                                                                        \
      end_trace();                                                             \
    }                                                                          \
    start_trace(4, tree_get_node(tree, tree.root)->hash,                       \
                trace_span("Setting root to black"));                          \
    tree_write_color(tree, tree.root, NODE_COLOR_BLACK);                       \
    end_trace();                                                               \
    end_trace();                                                               \
  } while (0)

#define tree_rb_insert_fixup_dir(tree, node_addr, f_branch, f_direction)       \
  do {                                                                         \
    tree_node_t *node = tree_get_node(tree, node_addr);                        \
    tree_addr_t parent_addr = node->parent;                                    \
                                                                               \
    tree_node_t *parent = tree_get_node(tree, parent_addr);                    \
    tree_addr_t grandparent_addr = parent->parent;                             \
                                                                               \
    tree_node_t *grandparent = tree_get_node(tree, grandparent_addr);          \
    tree_addr_t uncle_addr = grandparent->f_branch;                            \
                                                                               \
    if (tree_is_red(tree, uncle_addr) == NODE_COLOR_RED) {                     \
      start_trace(5, node->hash, trace_span("Uncle is red."));                 \
                                                                               \
      trace(trace_result("Setting parent to black."));                         \
      tree_write_color(tree, parent_addr, NODE_COLOR_BLACK);                   \
      trace(trace_result("Setting uncle to black."));                          \
      tree_write_color(tree, uncle_addr, NODE_COLOR_BLACK);                    \
      trace(trace_result("Setting grandparent to red."));                      \
      tree_write_color(tree, grandparent_addr, NODE_COLOR_RED);                \
                                                                               \
      node_addr = grandparent_addr;                                            \
                                                                               \
      end_trace();                                                             \
    } else {                                                                   \
      start_trace(6, node->hash, trace_span("Uncle is black."));               \
                                                                               \
      if (node == tree_get_sibling(tree, node, f_branch)) {                    \
        start_trace(                                                           \
            7, node->hash,                                                     \
            trace_span("Triangle case (node is aligned in the opposite way "   \
                       "as its parent)"));                                     \
                                                                               \
        node_addr = node->parent;                                              \
                                                                               \
        node = tree_get_node(tree, node_addr);                                 \
                                                                               \
        trace(trace_result("Updating node pointer to parent: %lld"),           \
              node->hash);                                                     \
        set_trace_span(node->hash);                                            \
        start_trace(8, node->hash,                                             \
                    trace_span("Rotating node to " #f_direction));             \
                                                                               \
        tree_rot(tree, node_addr, f_branch, f_direction);                      \
                                                                               \
        parent_addr = node->parent;                                            \
                                                                               \
        parent = tree_get_node(tree, parent_addr);                             \
                                                                               \
        end_trace();                                                           \
        end_trace();                                                           \
      }                                                                        \
                                                                               \
      assert(parent->parent != 0);                                             \
                                                                               \
      start_trace(                                                             \
          9, node->hash,                                                       \
          trace_span("Line case (node is aligned same way as its parent)"));   \
                                                                               \
      trace(trace_result("Setting parent color to black"));                    \
                                                                               \
      tree_write_color(tree, parent_addr, NODE_COLOR_BLACK);                   \
                                                                               \
      trace(trace_result("Setting grandparent color to red"));                 \
                                                                               \
      tree_write_color(tree, parent->parent, NODE_COLOR_RED);                  \
                                                                               \
      start_trace(10, tree_get_node(tree, parent->parent)->hash,               \
                  trace_span("Rotating grandparent of %lld to " #f_branch),    \
                  node->hash);                                                 \
                                                                               \
      tree_rot(tree, parent->parent, f_direction, f_branch);                   \
                                                                               \
      end_trace();                                                             \
      end_trace();                                                             \
      end_trace();                                                             \
    }                                                                          \
  } while (0)

#define tree_rb_insert_fixup_left(tree, node_addr)                             \
  tree_rb_insert_fixup_dir(tree, node_addr, right, left)

#define tree_rb_insert_fixup_right(tree, node_addr)                            \
  tree_rb_insert_fixup_dir(tree, node_addr, left, right)

#define tree_read_bitval(tree, addr, f_member)                                 \
  ({                                                                           \
    tree_idx_t idx = tree_idx(addr);                                           \
    tree_idx_t byte_idx = floor((float)idx / 8);                               \
    tree_idx_t bit_idx = idx % 8;                                              \
    uint8_t mask = 1 << bit_idx;                                               \
    (tree.f_member[byte_idx] & mask) != 0;                                     \
  })

#define tree_remove(tree, entry, find_node_entry, clear_entry)                 \
  do {                                                                         \
    uint64_t hash = tree.hash_fn(entry);                                       \
    start_trace(11, hash, trace_span("Removing entry %lld"), hash);            \
    tree_addr_t node_addr = find_node_entry(tree, hash, entry);                \
                                                                               \
    if (!tree_is_valid_addr(node_addr)) {                                      \
      trace(trace_info("Entry does not exist in tree"));                       \
      end_trace();                                                             \
      break;                                                                   \
    }                                                                          \
                                                                               \
    tree_node_t *node = tree_get_node(tree, node_addr);                        \
                                                                               \
    tree_addr_t color_sample_addr = node_addr;                                 \
    tree_addr_t fixup_target_addr;                                             \
                                                                               \
    bool color_sample_is_red = tree_is_red(tree, node_addr);                   \
                                                                               \
    if (!tree_is_inited(tree, node->left)) {                                   \
      trace(trace_info(                                                        \
          "Left child is NIL node or both children are NIL nodes"));           \
      tree_node_t *right = tree_get_node(tree, node->right);                   \
      trace(trace_result("Setting fixup target to right child: %lld"),         \
            right->hash);                                                      \
      fixup_target_addr = node->right;                                         \
      start_trace(12, node_addr,                                               \
                  trace_span("Transplanting right child (node %lld) here"),    \
                  right->hash);                                                \
      tree_transplant(tree, node_addr, node->right);                           \
      end_trace();                                                             \
    } else if (!tree_is_inited(tree, node->right)) {                           \
      trace(trace_info("Only right child is NIL node"));                       \
      tree_node_t *left = tree_get_node(tree, node->left);                     \
      trace(trace_result("Setting fixup target to left child: %lld"),          \
            left->hash);                                                       \
      fixup_target_addr = node->left;                                          \
      start_trace(13, node_addr,                                               \
                  trace_span("Transplanting left child (node %lld) here"),     \
                  left->hash);                                                 \
      tree_transplant(tree, node_addr, node->left);                            \
      end_trace();                                                             \
    } else {                                                                   \
      trace(trace_info("Neither of children are NIL nodes"));                  \
      assert(tree_is_inited(tree, node->right));                               \
      color_sample_addr = tree_min_in_branch(tree, node->right);               \
      tree_node_t *color_sample = tree_get_node(tree, color_sample_addr);      \
      color_sample_is_red = tree_is_red(tree, color_sample_addr);              \
      trace(trace_info("Found color sample as minimum of right child: %lld - " \
                       "color is %s"),                                         \
            color_sample->hash, color_sample_is_red ? "red" : "black");        \
      fixup_target_addr = color_sample->right;                                 \
      tree_node_t *fixup_target = tree_get_node(tree, fixup_target_addr);      \
      trace(trace_result(                                                      \
                "Setting fixup target to right child of color sample: %lld"),  \
            fixup_target->hash);                                               \
                                                                               \
      if (color_sample_addr == node->right) {                                  \
        trace(trace_info("Color sample is right child of node"));              \
        trace(                                                                 \
            trace_result(                                                      \
                "Setting fixup target (%lld) parent to color sample (%lld)"),  \
            fixup_target->hash, color_sample->hash);                           \
        fixup_target->parent = color_sample_addr;                              \
      } else {                                                                 \
        trace(trace_info("Color sample is not right child of node"));          \
        start_trace(14, color_sample->hash,                                    \
                    trace_span("Transplanting %lld to color sample (%lld)"),   \
                    fixup_target->hash, color_sample->hash);                   \
        tree_transplant(tree, color_sample_addr, fixup_target_addr);           \
        end_trace();                                                           \
        trace(trace_result(                                                    \
                  "2-way binding right child of color sample (%lld) to "       \
                  "right child of node (%lld)"),                               \
              color_sample->hash, node->hash);                                 \
        color_sample->right = node->right;                                     \
        tree_get_node(tree, color_sample->right)->parent = color_sample_addr;  \
      }                                                                        \
                                                                               \
      start_trace(                                                             \
          15, node->hash,                                                      \
          trace_span("Transplanting color sample (%lld) to node (%lld)"),      \
          color_sample->hash, node->hash);                                     \
      tree_transplant(tree, node_addr, color_sample_addr);                     \
      end_trace();                                                             \
                                                                               \
      trace(trace_result(                                                      \
                "2-way binding left child of color sample (%lld) to left "     \
                "child of node (%lld)"),                                       \
            color_sample->hash, node->hash);                                   \
      color_sample->left = node->left;                                         \
      tree_get_node(tree, color_sample->left)->parent = color_sample_addr;     \
      trace(trace_result("Setting color of color sample (%lld) to %s"),        \
            color_sample->hash,                                                \
            tree_is_red(tree, node_addr) ? "red" : "black");                   \
      tree_write_color(tree, color_sample_addr, tree_is_red(tree, node_addr)); \
    }                                                                          \
                                                                               \
    if (!color_sample_is_red) {                                                \
      trace(trace_info("Original color of color sample was black, fixing up "  \
                       "fix up target"));                                      \
      start_trace(16, fixup_target_addr,                                       \
                  trace_span("Fixing up fixup target"));                       \
      tree_delete_fixup(tree, fixup_target_addr);                              \
      end_trace();                                                             \
    } else {                                                                   \
      trace(trace_info(                                                        \
          "Original color of color sample was red, no fixup required"));       \
    }                                                                          \
                                                                               \
    tree_collision_t *collision = tree_get_collision(tree, node_addr);         \
    tree_addr_t collision_next = collision->next;                              \
    tree_addr_t collision_prev = collision->prev;                              \
    if (tree_is_valid_addr(collision_prev)) {                                  \
      tree_get_collision(tree, collision_prev)->next = collision_next;         \
    }                                                                          \
    if (tree_is_valid_addr(collision_next)) {                                  \
      tree_get_collision(tree, collision_next)->prev = collision_prev;         \
    }                                                                          \
                                                                               \
    trace(trace_result("Freeing node"));                                       \
    tree_free_node(tree, node_addr);                                           \
    trace(trace_result("Clearing entry"));                                     \
    clear_entry(tree, node_addr);                                              \
                                                                               \
    end_trace();                                                               \
  } while (0)

#define tree_rot(tree, node_addr, f_branch, f_direction)                       \
  do {                                                                         \
    tree_addr_t n_addr = (node_addr);                                          \
    const char *align_branch = #f_branch;                                      \
    const char *align_direction = #f_direction;                                \
    tree_node_t *rot_node = tree_get_node(tree, n_addr);                       \
    tree_addr_t f_branch_addr = rot_node->f_branch;                            \
    assert(tree_is_valid_addr(f_branch_addr));                                 \
    tree_node_t *f_branch = tree_get_node(tree, f_branch_addr);                \
                                                                               \
    trace(trace_result("Binding %s field to %s child of %s child"),            \
          align_branch, align_direction, align_branch);                        \
    rot_node->f_branch = f_branch->f_direction;                                \
    if (tree_is_valid_addr(f_branch->f_direction)) {                           \
      tree_get_node(tree, f_branch->f_direction)->parent = n_addr;             \
    }                                                                          \
                                                                               \
    f_branch->parent = rot_node->parent;                                       \
    if (!tree_is_valid_addr(rot_node->parent)) {                               \
      trace(trace_result("Binding root to %s child"), align_branch);           \
      tree.root = f_branch_addr;                                               \
    } else if (rot_node == tree_get_sibling(tree, rot_node, f_direction)) {    \
      trace(trace_result("Binding %s field of parent to %s child"),            \
            align_direction, align_branch);                                    \
      tree_get_node(tree, rot_node->parent)->f_direction = f_branch_addr;      \
    } else {                                                                   \
      trace(trace_result("Binding %s field of parent to %s child"),            \
            align_branch, align_branch);                                       \
      tree_get_node(tree, rot_node->parent)->f_branch = f_branch_addr;         \
    }                                                                          \
                                                                               \
    trace(trace_result("Binding %s field of %s child to this node"),           \
          align_direction, align_branch);                                      \
    f_branch->f_direction = n_addr;                                            \
    rot_node->parent = f_branch_addr;                                          \
  } while (0)

#define tree_rot_left(tree, node_addr) tree_rot(tree, node_addr, right, left)
#define tree_rot_right(tree, node_addr) tree_rot(tree, node_addr, left, right)

#define tree_seq(tree, node_addr, f_branch, f_direction)                       \
  ({                                                                           \
    tree_addr_t addr =                                                         \
        tree_seq_in_branch(tree, node_addr, f_branch, f_direction);            \
                                                                               \
    if (!tree_is_valid_addr(addr)) {                                           \
      tree_addr_t scan_addr = node_addr;                                       \
      tree_addr_t next = tree_get_node(tree, node_addr)->parent;               \
      while (tree_is_valid_addr(next)) {                                       \
        tree_node_t *next_node = tree_get_node(tree, next);                    \
        if (next_node->f_direction == scan_addr) {                             \
          addr = next;                                                         \
          break;                                                               \
        }                                                                      \
        scan_addr = next;                                                      \
        next = next_node->parent;                                              \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (!tree_is_valid_addr(addr)) {                                           \
      addr = 0;                                                                \
    }                                                                          \
                                                                               \
    addr;                                                                      \
  })

#define tree_seq_in_branch(tree, node_addr, f_branch, f_direction)             \
  ({                                                                           \
    tree_addr_t idx = 0;                                                       \
    if (tree_is_inited(tree, node_addr) == 1) {                                \
      tree_node_t *node = tree_get_node(tree, node_addr);                      \
                                                                               \
      if (tree_is_inited(tree, node->f_branch) == 1) {                         \
        idx = tree_ult_in_branch(tree, node->f_branch, f_direction);           \
      }                                                                        \
    }                                                                          \
    idx;                                                                       \
  })

#define tree_size(tree)                                                        \
  ({                                                                           \
    tree_addr_t cursor = tree_first(tree);                                     \
    size_t size = 0;                                                           \
    while (tree_is_valid_addr(cursor) && tree_is_inited(tree, cursor)) {       \
      size++;                                                                  \
      cursor = tree_next(tree, cursor);                                        \
    }                                                                          \
    size;                                                                      \
  })

#define tree_transplant(tree, dest_addr, src_addr)                             \
  do {                                                                         \
    tree_node_t *dest = tree_get_node(tree, dest_addr);                        \
    if (!tree_is_valid_addr(dest->parent)) {                                   \
      tree.root = src_addr;                                                    \
    } else {                                                                   \
      tree_node_t *parent = tree_get_node(tree, dest->parent);                 \
      if (dest_addr == parent->left) {                                         \
        parent->left = src_addr;                                               \
      } else {                                                                 \
        parent->right = src_addr;                                              \
      }                                                                        \
    }                                                                          \
    tree_node_t *src = tree_get_node(tree, src_addr);                          \
    src->parent = dest->parent;                                                \
  } while (0)

#define tree_type_fields()                                                     \
  tree_addr_t *free_list;                                                      \
  tree_addr_t free_list_start;                                                 \
  tree_addr_t root;                                                            \
  uint8_t *colors;                                                             \
  uint8_t *inited;                                                             \
  tree_node_t *nodes;                                                          \
  tree_collision_t *collisions;                                                \
  size_t count;                                                                \
  size_t capacity;

#define tree_ult(tree, f_direction)                                            \
  ({                                                                           \
    tree_addr_t cursor = tree.root;                                            \
    tree_addr_t retval = 0;                                                    \
    while (tree_is_valid_addr(cursor) && tree_is_inited(tree, cursor)) {       \
      retval = cursor;                                                         \
      cursor = tree_get_node(tree, cursor)->f_direction;                       \
    }                                                                          \
    retval;                                                                    \
  })

#define tree_ult_in_branch(tree, node_addr, f_direction)                       \
  ({                                                                           \
    tree_addr_t idx = (node_addr);                                             \
    tree_node_t *node;                                                         \
    while (true) {                                                             \
      node = tree_get_node(tree, idx);                                         \
      if (!tree_is_inited(tree, node->f_direction)) {                          \
        break;                                                                 \
      }                                                                        \
      idx = node->f_direction;                                                 \
    }                                                                          \
    idx;                                                                       \
  })

#define tree_write_bitval(tree, addr, f_member, val)                           \
  do {                                                                         \
    tree_idx_t idx = tree_idx(addr);                                           \
    tree_idx_t byte_idx = floor((float)idx / 8);                               \
    tree_idx_t bit_idx = idx % 8;                                              \
    uint8_t mask = 1 << bit_idx;                                               \
    if (val == 1) {                                                            \
      tree.f_member[byte_idx] |= mask;                                         \
    } else {                                                                   \
      tree.f_member[byte_idx] &= ~mask;                                        \
    }                                                                          \
  } while (0)

#define tree_write_color(tree, addr, val)                                      \
  tree_write_bitval(tree, addr, colors, val)

#define tree_write_inited(tree, addr, val)                                     \
  tree_write_bitval(tree, addr, inited, val)

#endif // !GENERIC_SET_H
