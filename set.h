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

#define COLLISION_NIL                                                          \
  { .next = 0, .prev = 0 }

typedef struct {
  size_t left;
  size_t right;
  size_t parent;
  uint64_t hash;
} tree_node_t;

typedef struct {
  size_t next;
  size_t prev;
} tree_collision_t;

#define ALLOC_CHUNK 512

#define BIT_HEADER_ADDR ((size_t)1 << 63)
#define BIT_HEADER_IDX (~BIT_HEADER_ADDR)

/* Actually freeing and remallocing seems like a really expensive way to
 * do this, let's try to find something better */

#define map_add(map, key_var, value_var)                                       \
  do {                                                                         \
    size_t leaf_idx = tree_add(map, key_var, map_alloc_new_node,               \
                               map_write_key, map_find_duplicate);             \
                                                                               \
    if (tree_is_valid_addr(leaf_idx)) {                                        \
      map_write_value(map, leaf_idx, value_var);                               \
    }                                                                          \
  } while (0)

#define map_alloc_new_node(map)                                                \
  tree_alloc_new_node(map, map_create_entry, map_realloc_entries)

#define map_clear_entry(map, addr)                                             \
  do {                                                                         \
    size_t clear_idx = tree_idx(addr);                                         \
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

#define map_find_duplicate(map, node_idx, key_var)                             \
  tree_find_duplicate(map, node_idx, key_var, map_get_key, keys)

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
    size_t node_idx = map_find_node_entry(map, hash, key);                     \
    typeof(map.values) retval = NULL;                                          \
    if (tree_is_valid_addr(node_idx)) {                                        \
      size_t idx = tree_idx(node_idx);                                         \
      assert(map.capacity > idx);                                              \
      retval = &map.values[idx];                                               \
    }                                                                          \
    retval;                                                                    \
  })

#define map_get_key(map, addr)                                                 \
  ({                                                                           \
    size_t idx = tree_idx(addr);                                               \
    assert(map.capacity > idx);                                                \
    map.keys[idx];                                                             \
  })

#define map_get_value(map, addr)                                               \
  ({                                                                           \
    size_t idx = tree_idx(addr);                                               \
    assert(map.capacity > idx);                                                \
    map.values[idx];                                                           \
  })

#define map_has(map, key)                                                      \
  ({                                                                           \
    uint64_t hash = map.hash_fn(key);                                          \
    size_t node_idx = map_find_node_entry(map, hash, key);                     \
    tree_is_valid_addr(node_idx);                                              \
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
    size_t map_write_key_idx = tree_idx(addr);                                 \
    assert(map.capacity > map_write_key_idx);                                  \
    map.keys[map_write_key_idx] = key;                                         \
    tree_write_inited(map, addr, true);                                        \
  } while (0)

#define map_write_value(map, addr, value)                                      \
  do {                                                                         \
    size_t map_write_value_idx = tree_idx(addr);                               \
    assert(map.capacity > map_write_value_idx);                                \
    map.values[map_write_value_idx] = value;                                   \
    tree_write_inited(map, addr, true);                                        \
  } while (0)

#define set_add(set, entry_var)                                                \
  tree_add(set, entry_var, set_alloc_new_node, set_write_entry,                \
           set_find_duplicate)

#define set_alloc_new_node(set)                                                \
  tree_alloc_new_node(set, set_create_entry, set_realloc_entries)

#define set_clear_entry(set, addr)                                             \
  do {                                                                         \
    size_t clear_idx = tree_idx(addr);                                         \
    assert(set.capacity > clear_idx);                                          \
    memset(&set.entries[clear_idx], 0x00, sizeof(typeof(set.entries)));        \
    tree_write_inited(set, addr, false);                                       \
  } while (0)

#define set_clone(set)                                                         \
  tree_clone(set, set_malloc_entries, set_get_entry, set_write_entry)

#define set_create_entry(set, idx)                                             \
  memset(&set.entries[idx], 0x00, sizeof(typeof(*set.entries)))

#define set_empty(set) tree_empty(set, set_init, set_free)

#define set_find_duplicate(set, node_idx, entry_var)                           \
  tree_find_duplicate(set, node_idx, entry_var, set_get_entry, entries)

#define set_find_node_entry(set, hash_value, entry_var)                        \
  tree_find_node_entry(set, hash_value, entry_var, set_find_duplicate)

#define set_free(set) tree_free(set, set_free_data)

#define set_free_data(set)                                                     \
  do {                                                                         \
    free(set.entries);                                                         \
  } while (0)

#define set_get_entry(set, addr)                                               \
  ({                                                                           \
    size_t idx = tree_idx(addr);                                               \
    assert(set.capacity > idx);                                                \
    set.entries[idx];                                                          \
  })

#define set_has(set, entry)                                                    \
  ({                                                                           \
    uint64_t hash = set.hash_fn(entry);                                        \
    size_t node_idx = set_find_node_entry(set, hash, entry);                   \
    tree_is_valid_addr(node_idx);                                              \
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
    size_t set_write_entry_idx = tree_idx(addr);                               \
    assert(set.capacity > set_write_entry_idx);                                \
    set.entries[set_write_entry_idx] = entry;                                  \
    tree_write_inited(set, addr, true);                                        \
  } while (0)

#define tree_add(tree, entry_var, alloc_new_node, tree_write_entry,            \
                 find_duplicate)                                               \
  ({                                                                           \
    size_t retval = 0;                                                         \
    do {                                                                       \
      uint64_t hash = tree.hash_fn(entry_var);                                 \
      start_trace(hash, trace_span("Adding entry"));                           \
      size_t leaf_idx = tree_find_node(tree, hash);                            \
                                                                               \
      tree_node_t *leaf = tree_get_node(tree, leaf_idx);                       \
                                                                               \
      if (tree_is_inited(tree, leaf_idx) != 0) {                               \
        size_t duplicate_idx = find_duplicate(tree, leaf_idx, entry_var);      \
                                                                               \
        if (tree_is_valid_addr(duplicate_idx)) {                               \
          retval = 0;                                                          \
          trace(trace_info("Entry already exists in tree"));                   \
          end_trace();                                                         \
          break;                                                               \
        }                                                                      \
                                                                               \
        tree_collision_t *collision = tree_get_collision(tree, leaf_idx);      \
                                                                               \
        while (tree_is_valid_addr(collision->next)) {                          \
          leaf_idx = collision->next;                                          \
          collision = tree_get_collision(tree, leaf_idx);                      \
        }                                                                      \
        size_t next;                                                           \
        size_t n = tree_get_node(tree, leaf_idx)->right;                       \
        while (tree_is_valid_addr(n)) {                                        \
          next = n;                                                            \
          n = tree_get_node(tree, next)->left;                                 \
        }                                                                      \
        collision->next = next;                                                \
        tree_get_collision(tree, next)->prev = leaf_idx;                       \
        leaf_idx = next;                                                       \
        leaf = tree_get_node(tree, leaf_idx);                                  \
      }                                                                        \
                                                                               \
      size_t left_idx = alloc_new_node(tree);                                  \
      size_t right_idx = alloc_new_node(tree);                                 \
      tree_get_node(tree, left_idx)->parent = leaf_idx;                        \
      tree_get_node(tree, right_idx)->parent = leaf_idx;                       \
      tree_write_color(tree, left_idx, NODE_COLOR_BLACK);                      \
      tree_write_color(tree, right_idx, NODE_COLOR_BLACK);                     \
                                                                               \
      *leaf = (tree_node_t){                                                   \
          .hash = hash,                                                        \
          .left = left_idx,                                                    \
          .parent = leaf->parent,                                              \
          .right = right_idx,                                                  \
      };                                                                       \
                                                                               \
      tree_write_entry(tree, leaf_idx, entry_var);                             \
      tree_write_color(tree, leaf_idx, NODE_COLOR_RED);                        \
                                                                               \
      tree_rb_insert_fixup(tree, leaf_idx);                                    \
      end_trace();                                                             \
                                                                               \
      retval = leaf_idx;                                                       \
                                                                               \
    } while (0);                                                               \
    retval;                                                                    \
  })

#define tree_addr(idx) (idx) + 1

#define tree_alloc_new_node(set, create_entry, realloc_entries)                \
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
        set.nodes[i] = (tree_node_t)NODE_NIL;                                  \
        set.collisions[i] = (tree_collision_t)COLLISION_NIL;                   \
        create_entry(set, i);                                                  \
        retval = tree_addr(i);                                                 \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if (!found_free) {                                                         \
      size_t i = set.capacity;                                                 \
      retval = tree_addr(i);                                                   \
      set.capacity *= 2;                                                       \
      set.nodes = realloc(set.nodes, sizeof(tree_node_t) * set.capacity);      \
      set.collisions =                                                         \
          realloc(set.collisions, sizeof(tree_collision_t) * set.capacity);    \
      realloc_entries(set);                                                    \
      set.free_list = realloc(set.free_list, set.capacity / 8);                \
      set.colors = realloc(set.colors, set.capacity / 8);                      \
      set.inited = realloc(set.inited, set.capacity / 8);                      \
      memset(&set.free_list[i / 8], 0xff, (set.capacity / 8) - (i / 8));       \
      memset(&set.colors[i / 8], 0x00, (set.capacity / 8) - (i / 8));          \
      memset(&set.inited[i / 8], 0x00, (set.capacity / 8) - (i / 8));          \
      set.nodes[i] = (tree_node_t)NODE_NIL;                                    \
      set.collisions[i] = (tree_collision_t)COLLISION_NIL;                     \
      create_entry(set, i);                                                    \
      set.free_list[i] = 0xff >> 1;                                            \
    }                                                                          \
    retval;                                                                    \
  })

#define tree_clone(tree, malloc_entries, get_entry, write_entry)               \
  ({                                                                           \
    typeof(tree) clone;                                                        \
    typeof(tree.hash_fn) hash_function = tree.hash_fn;                         \
    typeof(tree.equals_fn) equals_function = tree.equals_fn;                   \
                                                                               \
    clone.capacity = tree.capacity;                                            \
    clone.root = tree.root;                                                    \
    clone.nodes = malloc(sizeof(tree_node_t) * tree.capacity);                 \
    clone.collisions = malloc(sizeof(tree_collision_t) * tree.capacity);       \
    clone.entries = malloc(sizeof(typeof(*clone.entries)) * tree.capacity);    \
    clone.free_list = malloc(clone.capacity / 8);                              \
    clone.colors = malloc(clone.capacity / 8);                                 \
    clone.inited = malloc(clone.capacity / 8);                                 \
    malloc_entries(clone);                                                     \
                                                                               \
    memset(clone.free_list, 0xff, clone.capacity / 8);                         \
                                                                               \
    for (size_t i = 0; i < clone.capacity / 8; i++) {                          \
      if (tree.free_list[i] == 0xff) {                                         \
        continue;                                                              \
      }                                                                        \
                                                                               \
      clone.free_list[i] = tree.free_list[i];                                  \
      clone.colors[i] = tree.colors[i];                                        \
      clone.inited[i] = tree.inited[i];                                        \
                                                                               \
      for (size_t j = 0; j < 8; j++) {                                         \
        size_t slot = (i * 8) + j;                                             \
        clone.collisions[slot] = tree.collisions[slot];                        \
        clone.nodes[slot] = tree.nodes[slot];                                  \
        clone.entries[slot] = tree.entries[slot];                              \
        write_entry(clone, slot + 1, get_entry(tree, slot + 1));               \
      }                                                                        \
    }                                                                          \
                                                                               \
    clone.hash_fn = hash_function;                                             \
    clone.equals_fn = equals_function;                                         \
                                                                               \
    clone;                                                                     \
  })

#define tree_delete_fixup(tree, node_idx)                                      \
  do {                                                                         \
    while (node_idx != tree.root) {                                            \
      tree_node_t *node = tree_get_node(tree, node_idx);                       \
      start_trace(node->hash, trace_span("Fixing up %lld"), node->hash);       \
      bool color = tree_is_red(tree, node_idx);                                \
      if (color != NODE_COLOR_BLACK) {                                         \
        trace(trace_info("Node is red, stop traversing"));                     \
        end_trace();                                                           \
        break;                                                                 \
      }                                                                        \
      if (node == tree_get_sibling(tree, node, left)) {                        \
        trace(trace_info("Node is in parents left subtree"));                  \
        tree_delete_fixup_dir(tree, node_idx, right, left);                    \
      } else {                                                                 \
        tree_delete_fixup_dir(tree, node_idx, left, right);                    \
      }                                                                        \
      end_trace();                                                             \
    }                                                                          \
    tree_write_color(tree, node_idx, NODE_COLOR_BLACK);                        \
  } while (0)

#define tree_delete_fixup_dir(tree, node_idx, f_branch, f_direction)           \
  do {                                                                         \
    tree_node_t *node = tree_get_node(tree, node_idx);                         \
    tree_node_t *parent = tree_get_node(tree, node->parent);                   \
    size_t sibling_idx = parent->f_branch;                                     \
                                                                               \
    if (tree_is_red(tree, sibling_idx) == NODE_COLOR_RED) {                    \
      tree_write_color(tree, sibling_idx, NODE_COLOR_BLACK);                   \
      tree_write_color(tree, node->parent, NODE_COLOR_RED);                    \
      tree_rot(tree, node->parent, f_branch, f_direction);                     \
      sibling_idx = tree_get_node(tree, node->parent)->f_branch;               \
    }                                                                          \
                                                                               \
    tree_node_t *sibling = tree_get_node(tree, sibling_idx);                   \
                                                                               \
    if (tree_is_red(tree, sibling->f_branch) == NODE_COLOR_BLACK &&            \
        tree_is_red(tree, sibling->f_direction) == NODE_COLOR_BLACK) {         \
      tree_write_color(tree, sibling_idx, NODE_COLOR_RED);                     \
      node_idx = node->parent;                                                 \
      node = tree_get_node(tree, node_idx);                                    \
    } else {                                                                   \
      if (tree_is_red(tree, sibling->f_branch) == NODE_COLOR_BLACK) {          \
        tree_write_color(tree, sibling->f_direction, NODE_COLOR_BLACK);        \
        tree_write_color(tree, sibling_idx, NODE_COLOR_RED);                   \
        tree_rot(tree, sibling_idx, f_direction, f_branch);                    \
        sibling_idx =                                                          \
            tree_get_node(tree, tree_get_node(tree, node_idx)->parent)         \
                ->f_branch;                                                    \
        sibling = tree_get_node(tree, sibling_idx);                            \
      }                                                                        \
      tree_write_color(tree, sibling_idx, tree_is_red(tree, node->parent));    \
      tree_write_color(tree, node->parent, NODE_COLOR_BLACK);                  \
      tree_write_color(tree, sibling->f_branch, NODE_COLOR_BLACK);             \
      tree_rot(tree, node->parent, f_branch, f_direction);                     \
      node_idx = tree.root;                                                    \
    }                                                                          \
  } while (0)

#define tree_empty(tree, set_init, set_free)                                   \
  do {                                                                         \
    typeof(tree.hash_fn) hash_fn = tree.hash_fn;                               \
    typeof(tree.equals_fn) equals_fn = tree.equals_fn;                         \
    set_free(tree);                                                            \
    set_init(tree, hash_fn, equals_fn);                                        \
  } while (0)

#define tree_find_duplicate(tree, node_idx, entry_var, tree_get_entry,         \
                            f_entries)                                         \
  ({                                                                           \
    typeof(*tree.f_entries) entry_val = (entry_var);                           \
    typeof(*tree.f_entries) entry = tree_get_entry(tree, node_idx);            \
                                                                               \
    size_t retval = 0;                                                         \
                                                                               \
    do {                                                                       \
      if (tree.equals_fn(entry, entry_val)) {                                  \
        retval = node_idx;                                                     \
        break;                                                                 \
      }                                                                        \
                                                                               \
      tree_collision_t *collision = tree_get_collision(tree, node_idx);        \
      tree_collision_t *c = collision;                                         \
      while (tree_is_valid_addr(c->prev)) {                                    \
        entry = tree_get_entry(tree, c->prev);                                 \
        if (tree.equals_fn(entry, entry_val)) {                                \
          retval = c->prev;                                                    \
          break;                                                               \
        }                                                                      \
        c = tree_get_collision(tree, c->prev);                                 \
      }                                                                        \
      if (!tree_is_valid_addr(retval)) {                                       \
        c = collision;                                                         \
        while (tree_is_valid_addr(c->next)) {                                  \
          entry = tree_get_entry(tree, c->next);                               \
          if (tree.equals_fn(entry, entry_val)) {                              \
            retval = c->next;                                                  \
            break;                                                             \
          }                                                                    \
          c = tree_get_collision(tree, c->next);                               \
        }                                                                      \
      }                                                                        \
    } while (0);                                                               \
    retval;                                                                    \
  })

#define tree_find_node(tree, hash_value)                                       \
  ({                                                                           \
    size_t n_idx = tree.root;                                                  \
    size_t find_node_retval;                                                   \
    while (tree_is_valid_addr(n_idx)) {                                        \
      find_node_retval = n_idx;                                                \
      tree_node_t *node = tree_get_node(tree, n_idx);                          \
      if ((hash_value) > node->hash) {                                         \
        n_idx = node->right;                                                   \
      } else if ((hash_value) < node->hash) {                                  \
        n_idx = node->left;                                                    \
      } else {                                                                 \
        break;                                                                 \
      }                                                                        \
    };                                                                         \
    find_node_retval;                                                          \
  })

#define tree_find_node_entry(tree, hash_value, entry_var, tree_find_duplicate) \
  ({                                                                           \
    uint64_t hash_val = (hash_value);                                          \
    size_t node_idx = tree_find_node(tree, hash_val);                          \
    size_t retval = 0;                                                         \
    if (tree_is_inited(tree, node_idx)) {                                      \
      retval = tree_find_duplicate(tree, node_idx, entry_var);                 \
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
    size_t idx = tree_idx(addr);                                               \
    size_t free_list_byte_idx = floor((float)idx / 8);                         \
    size_t free_list_bit_idx = idx % 8;                                        \
    uint8_t bmask = 1 << (7 - free_list_bit_idx);                              \
    tree.free_list[free_list_byte_idx] ^= bmask;                               \
  } while (0)

#define tree_get_collision(tree, addr)                                         \
  ({                                                                           \
    size_t idx = tree_idx(addr);                                               \
    if (tree.capacity <= idx) {                                                \
      printf(                                                                  \
          "Warning: tree capacity (%zu) is less than or equal to index %zu\n", \
          tree.capacity, idx);                                                 \
    }                                                                          \
    assert(tree.capacity > idx);                                               \
    &tree.collisions[idx];                                                     \
  })

#define tree_get_node(tree, addr)                                              \
  ({                                                                           \
    size_t a = (addr);                                                         \
    tree_node_t *retval = NULL;                                                \
    if (tree_is_valid_addr(a)) {                                               \
      size_t idx = tree_idx(a);                                                \
      assert(tree.capacity > idx);                                             \
      retval = &tree.nodes[idx];                                               \
    }                                                                          \
    retval;                                                                    \
  })

#define tree_get_sibling(tree, node, f_branch)                                 \
  tree_get_node(tree, tree_get_node(tree, node->parent)->f_branch)

#define tree_idx(addr) (addr) - 1

#define tree_init(tree, hash_function, equals_function, malloc_entries,        \
                  alloc_new_node)                                              \
  do {                                                                         \
    tree.capacity = ALLOC_CHUNK;                                               \
    tree.nodes = malloc(sizeof(tree_node_t) * tree.capacity);                  \
    tree.collisions = malloc(sizeof(tree_collision_t) * tree.capacity);        \
    tree.free_list = malloc(tree.capacity / 8);                                \
    tree.colors = malloc(tree.capacity / 8);                                   \
    tree.inited = malloc(tree.capacity / 8);                                   \
    malloc_entries(tree);                                                      \
    memset(tree.free_list, 0xff, tree.capacity / 8);                           \
    memset(tree.colors, 0x00, tree.capacity / 8);                              \
    memset(tree.inited, 0x00, tree.capacity / 8);                              \
    tree.root = alloc_new_node(tree);                                          \
    tree.hash_fn = hash_function;                                              \
    tree.equals_fn = equals_function;                                          \
  } while (0)

#define tree_is_inited(set, addr) tree_read_bitval(set, addr, inited)
#define tree_is_red(set, addr) tree_read_bitval(set, addr, colors)
#define tree_is_valid_addr(addr) ((size_t)addr != 0)

#define tree_last(set) tree_ult(set, right)

#define tree_max_in_branch(set, node_idx)                                      \
  tree_ult_in_branch(set, node_idx, right);
#define tree_min_in_branch(set, node_idx)                                      \
  tree_ult_in_branch(set, node_idx, left);

#define tree_next(set, node_idx) tree_seq(set, node_idx, right, left)
#define tree_next_in_branch(set, node_idx)                                     \
  tree_seq_in_branch(set, node_idx, right, left);

#define tree_prev(set, node_idx) tree_seq(set, node_idx, left, right)
#define tree_prev_in_branch(set, node_idx)                                     \
  tree_seq_in_branch(set, node_idx, left, right);

#define tree_rb_insert_fixup(set, node_idx)                                    \
  do {                                                                         \
    while (true) {                                                             \
      tree_node_t *node = tree_get_node(set, node_idx);                        \
      start_trace(node->hash, trace_span("Fixing up node %lld"), node->hash);  \
      tree_node_t *parent = tree_get_node(set, node->parent);                  \
      if (parent == NULL ||                                                    \
          tree_is_red(set, node->parent) != NODE_COLOR_RED) {                  \
        trace(trace_info("Parent is NULL or black, doing nothing"));           \
        end_trace();                                                           \
        break;                                                                 \
      }                                                                        \
      if (parent == tree_get_sibling(set, parent, left)) {                     \
        trace(trace_info(                                                      \
            "Parent is in its parents left tree, doing fixup_left"));          \
        tree_rb_insert_fixup_left(set, node_idx);                              \
      } else {                                                                 \
        trace(trace_info("Parent is in its parents right tree, "               \
                         "doing fixup_right"));                                \
        tree_rb_insert_fixup_right(set, node_idx);                             \
      }                                                                        \
      end_trace();                                                             \
    }                                                                          \
    start_trace(tree_get_node(set, set.root)->hash,                            \
                trace_span("Setting root to black"));                          \
    tree_write_color(set, set.root, NODE_COLOR_BLACK);                         \
    end_trace();                                                               \
  } while (0)

#define tree_rb_insert_fixup_dir(set, node_idx, f_branch, f_direction)         \
  do {                                                                         \
    tree_node_t *node = tree_get_node(set, node_idx);                          \
    size_t parent_idx = node->parent;                                          \
                                                                               \
    tree_node_t *parent = tree_get_node(set, parent_idx);                      \
    size_t grandparent_idx = parent->parent;                                   \
                                                                               \
    tree_node_t *grandparent = tree_get_node(set, grandparent_idx);            \
    size_t uncle_idx = grandparent->f_branch;                                  \
                                                                               \
    if (tree_is_red(set, uncle_idx) == NODE_COLOR_RED) {                       \
      start_trace(node->hash, trace_span("Uncle is red."));                    \
                                                                               \
      trace(trace_result("Setting parent to black."));                         \
      tree_write_color(set, parent_idx, NODE_COLOR_BLACK);                     \
      trace(trace_result("Setting uncle to black."));                          \
      tree_write_color(set, uncle_idx, NODE_COLOR_BLACK);                      \
      trace(trace_result("Setting grandparent to red."));                      \
      tree_write_color(set, grandparent_idx, NODE_COLOR_RED);                  \
                                                                               \
      node_idx = grandparent_idx;                                              \
                                                                               \
      end_trace();                                                             \
    } else {                                                                   \
      start_trace(node->hash, trace_span("Uncle is black."));                  \
                                                                               \
      if (node == tree_get_sibling(set, node, f_branch)) {                     \
        start_trace(                                                           \
            node->hash,                                                        \
            trace_span("Triangle case (node is aligned in the opposite way "   \
                       "as its parent)"));                                     \
                                                                               \
        node_idx = node->parent;                                               \
                                                                               \
        node = tree_get_node(set, node_idx);                                   \
                                                                               \
        trace(trace_result("Updating node pointer to parent: %lld"),           \
              node->hash);                                                     \
        set_trace_span(node->hash);                                            \
        start_trace(node->hash, trace_span("Rotating node to " #f_direction)); \
                                                                               \
        tree_rot(set, node_idx, f_branch, f_direction);                        \
                                                                               \
        parent_idx = node->parent;                                             \
                                                                               \
        parent = tree_get_node(set, parent_idx);                               \
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
      tree_write_color(set, parent_idx, NODE_COLOR_BLACK);                     \
                                                                               \
      trace(trace_result("Setting grandparent color to red"));                 \
                                                                               \
      tree_write_color(set, parent->parent, NODE_COLOR_RED);                   \
                                                                               \
      start_trace(tree_get_node(set, parent->parent)->hash,                    \
                  trace_span("Rotating grandparent of %lld to " #f_branch),    \
                  node->hash);                                                 \
                                                                               \
      tree_rot(set, parent->parent, f_direction, f_branch);                    \
                                                                               \
      end_trace();                                                             \
      end_trace();                                                             \
    }                                                                          \
  } while (0)

#define tree_rb_insert_fixup_left(set, node_idx)                               \
  tree_rb_insert_fixup_dir(set, node_idx, right, left)

#define tree_rb_insert_fixup_right(set, node_idx)                              \
  tree_rb_insert_fixup_dir(set, node_idx, left, right)

#define tree_read_bitval(set, addr, f_member)                                  \
  ({                                                                           \
    size_t idx = tree_idx(addr);                                               \
    size_t byte_idx = floor((float)idx / 8);                                   \
    size_t bit_idx = idx % 8;                                                  \
    uint8_t mask = 1 << (7 - bit_idx);                                         \
    (set.f_member[byte_idx] & mask) != 0;                                      \
  })

#define tree_remove(tree, entry, find_node_entry, clear_entry)                 \
  do {                                                                         \
    uint64_t hash = tree.hash_fn(entry);                                       \
    start_trace(hash, trace_span("Removing entry %lld"), hash);                \
    size_t node_idx = find_node_entry(tree, hash, entry);                      \
                                                                               \
    if (!tree_is_valid_addr(node_idx)) {                                       \
      trace(trace_info("Entry does not exist in tree"));                       \
      end_trace();                                                             \
      break;                                                                   \
    }                                                                          \
                                                                               \
    tree_node_t *node = tree_get_node(tree, node_idx);                         \
                                                                               \
    size_t color_sample_idx = node_idx;                                        \
    size_t fixup_target_idx;                                                   \
                                                                               \
    bool color_sample_is_red = tree_is_red(tree, node_idx);                    \
                                                                               \
    if (!tree_is_inited(tree, node->left)) {                                   \
      trace(trace_info(                                                        \
          "Left child is NIL node or both children are NIL nodes"));           \
      tree_node_t *right = tree_get_node(tree, node->right);                   \
      trace(trace_result("Setting fixup target to right child: %lld"),         \
            right->hash);                                                      \
      fixup_target_idx = node->right;                                          \
      start_trace(node_idx,                                                    \
                  trace_span("Transplanting right child (node %lld) here"),    \
                  right->hash);                                                \
      tree_transplant(tree, node_idx, node->right);                            \
      end_trace();                                                             \
    } else if (!tree_is_inited(tree, node->right)) {                           \
      trace(trace_info("Only right child is NIL node"));                       \
      tree_node_t *left = tree_get_node(tree, node->left);                     \
      trace(trace_result("Setting fixup target to left child: %lld"),          \
            left->hash);                                                       \
      fixup_target_idx = node->left;                                           \
      start_trace(node_idx,                                                    \
                  trace_span("Transplanting left child (node %lld) here"),     \
                  left->hash);                                                 \
      tree_transplant(tree, node_idx, node->left);                             \
      end_trace();                                                             \
    } else {                                                                   \
      trace(trace_info("Neither of children are NIL nodes"));                  \
      assert(tree_is_inited(tree, node->right));                               \
      color_sample_idx = tree_min_in_branch(tree, node->right);                \
      tree_node_t *color_sample = tree_get_node(tree, color_sample_idx);       \
      color_sample_is_red = tree_is_red(tree, color_sample_idx);               \
      trace(trace_info("Found color sample as minimum of right child: %lld - " \
                       "color is %s"),                                         \
            color_sample->hash, color_sample_is_red ? "red" : "black");        \
      fixup_target_idx = color_sample->right;                                  \
      tree_node_t *fixup_target = tree_get_node(tree, fixup_target_idx);       \
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
        tree_transplant(tree, color_sample_idx, fixup_target_idx);             \
        end_trace();                                                           \
        trace(trace_result(                                                    \
                  "2-way binding right child of color sample (%lld) to "       \
                  "right child of node (%lld)"),                               \
              color_sample->hash, node->hash);                                 \
        color_sample->right = node->right;                                     \
        tree_get_node(tree, color_sample->right)->parent = color_sample_idx;   \
      }                                                                        \
                                                                               \
      start_trace(                                                             \
          node->hash,                                                          \
          trace_span("Transplanting color sample (%lld) to node (%lld)"),      \
          color_sample->hash, node->hash);                                     \
      tree_transplant(tree, node_idx, color_sample_idx);                       \
      end_trace();                                                             \
                                                                               \
      trace(trace_result(                                                      \
                "2-way binding left child of color sample (%lld) to left "     \
                "child of node (%lld)"),                                       \
            color_sample->hash, node->hash);                                   \
      color_sample->left = node->left;                                         \
      tree_get_node(tree, color_sample->left)->parent = color_sample_idx;      \
      trace(trace_result("Setting color of color sample (%lld) to %s"),        \
            color_sample->hash,                                                \
            tree_is_red(tree, node_idx) ? "red" : "black");                    \
      tree_write_color(tree, color_sample_idx, tree_is_red(tree, node_idx));   \
    }                                                                          \
                                                                               \
    if (!color_sample_is_red) {                                                \
      trace(trace_info("Original color of color sample was black, fixing up "  \
                       "fix up target"));                                      \
      start_trace(fixup_target_idx, trace_span("Fixing up fixup target"));     \
      tree_delete_fixup(tree, fixup_target_idx);                               \
      end_trace();                                                             \
    } else {                                                                   \
      trace(trace_info(                                                        \
          "Original color of color sample was red, no fixup required"));       \
    }                                                                          \
                                                                               \
    tree_collision_t *collision = tree_get_collision(tree, node_idx);          \
    size_t collision_next = collision->next;                                   \
    size_t collision_prev = collision->prev;                                   \
    if (tree_is_valid_addr(collision_prev)) {                                  \
      tree_get_collision(tree, collision_prev)->next = collision_next;         \
    }                                                                          \
    if (tree_is_valid_addr(collision_next)) {                                  \
      tree_get_collision(tree, collision_next)->prev = collision_prev;         \
    }                                                                          \
                                                                               \
    trace(trace_result("Freeing node"));                                       \
    tree_free_node(tree, node_idx);                                            \
    trace(trace_result("Clearing entry"));                                     \
    clear_entry(tree, node_idx);                                               \
                                                                               \
    end_trace();                                                               \
  } while (0)

#define tree_rot(tree, node_idx, f_branch, f_direction)                        \
  do {                                                                         \
    size_t n_idx = (node_idx);                                                 \
    const char *align_branch = #f_branch;                                      \
    const char *align_direction = #f_direction;                                \
    tree_node_t *rot_node = tree_get_node(tree, n_idx);                        \
    size_t f_branch_idx = rot_node->f_branch;                                  \
    assert(tree_is_valid_addr(f_branch_idx));                                  \
    tree_node_t *f_branch = tree_get_node(tree, f_branch_idx);                 \
                                                                               \
    trace(trace_result("Binding %s field to %s child of %s child"),            \
          align_branch, align_direction, align_branch);                        \
    rot_node->f_branch = f_branch->f_direction;                                \
    if (tree_is_valid_addr(f_branch->f_direction)) {                           \
      tree_get_node(tree, f_branch->f_direction)->parent = n_idx;              \
    }                                                                          \
                                                                               \
    f_branch->parent = rot_node->parent;                                       \
    if (!tree_is_valid_addr(rot_node->parent)) {                               \
      trace(trace_result("Binding root to %s child"), align_branch);           \
      tree.root = f_branch_idx;                                                \
    } else if (rot_node == tree_get_sibling(tree, rot_node, f_direction)) {    \
      trace(trace_result("Binding %s field of parent to %s child"),            \
            align_direction, align_branch);                                    \
      tree_get_node(tree, rot_node->parent)->f_direction = f_branch_idx;       \
    } else {                                                                   \
      trace(trace_result("Binding %s field of parent to %s child"),            \
            align_branch, align_branch);                                       \
      tree_get_node(tree, rot_node->parent)->f_branch = f_branch_idx;          \
    }                                                                          \
                                                                               \
    trace(trace_result("Binding %s field of %s child to this node"),           \
          align_direction, align_branch);                                      \
    f_branch->f_direction = n_idx;                                             \
    rot_node->parent = f_branch_idx;                                           \
  } while (0)

#define tree_rot_left(tree, node_idx) tree_rot(tree, node_idx, right, left)
#define tree_rot_right(tree, node_idx) tree_rot(tree, node_idx, left, right)

#define tree_seq(tree, node_idx, f_branch, f_direction)                        \
  ({                                                                           \
    size_t addr = tree_seq_in_branch(tree, node_idx, f_branch, f_direction);   \
                                                                               \
    if (!tree_is_valid_addr(addr)) {                                           \
      size_t scan_addr = node_idx;                                             \
      size_t next = tree_get_node(tree, node_idx)->parent;                     \
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

#define tree_seq_in_branch(tree, node_idx, f_branch, f_direction)              \
  ({                                                                           \
    size_t idx = 0;                                                            \
    if (tree_is_inited(tree, node_idx) == 1) {                                 \
      tree_node_t *node = tree_get_node(tree, node_idx);                       \
                                                                               \
      if (tree_is_inited(tree, node->f_branch) == 1) {                         \
        idx = tree_ult_in_branch(tree, node->f_branch, f_direction);           \
      }                                                                        \
    }                                                                          \
    idx;                                                                       \
  })

#define tree_size(tree)                                                        \
  ({                                                                           \
    size_t cursor = tree_first(tree);                                          \
    size_t size = 0;                                                           \
    while (tree_is_valid_addr(cursor) && tree_is_inited(tree, cursor)) {       \
      size++;                                                                  \
      cursor = tree_next(tree, cursor);                                        \
    }                                                                          \
    size;                                                                      \
  })

#define tree_transplant(tree, dest_idx, src_idx)                               \
  do {                                                                         \
    tree_node_t *dest = tree_get_node(tree, dest_idx);                         \
    if (!tree_is_valid_addr(dest->parent)) {                                   \
      tree.root = src_idx;                                                     \
    } else {                                                                   \
      tree_node_t *parent = tree_get_node(tree, dest->parent);                 \
      if (dest_idx == parent->left) {                                          \
        parent->left = src_idx;                                                \
      } else {                                                                 \
        parent->right = src_idx;                                               \
      }                                                                        \
    }                                                                          \
    tree_node_t *src = tree_get_node(tree, src_idx);                           \
    src->parent = dest->parent;                                                \
  } while (0)

#define tree_type_fields()                                                     \
  tree_node_t *nodes;                                                          \
  tree_collision_t *collisions;                                                \
  size_t root;                                                                 \
  size_t capacity;                                                             \
  uint8_t *colors;                                                             \
  uint8_t *free_list;                                                          \
  uint8_t *inited;

#define tree_ult(tree, f_direction)                                            \
  ({                                                                           \
    size_t cursor = tree.root;                                                 \
    size_t retval = 0;                                                         \
    while (tree_is_valid_addr(cursor) && tree_is_inited(tree, cursor)) {       \
      retval = cursor;                                                         \
      cursor = tree_get_node(tree, cursor)->f_direction;                       \
    }                                                                          \
    retval;                                                                    \
  })

#define tree_ult_in_branch(tree, node_idx, f_direction)                        \
  ({                                                                           \
    size_t idx = (node_idx);                                                   \
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
    size_t idx = tree_idx(addr);                                               \
    size_t byte_idx = floor((float)idx / 8);                                   \
    size_t bit_idx = idx % 8;                                                  \
    uint8_t mask = 1 << (7 - bit_idx);                                         \
    if (val == 1) {                                                            \
      tree.f_member[byte_idx] |= mask;                                         \
    } else {                                                                   \
      tree.f_member[byte_idx] &= ~mask;                                        \
    }                                                                          \
  } while (0)

#define tree_write_color(tree, addr, val)                                      \
  tree_write_bitval(tree, addr, colors, val)

#define tree_write_inited(set, addr, val)                                      \
  tree_write_bitval(set, addr, inited, val)

#endif // !GENERIC_SET_H
