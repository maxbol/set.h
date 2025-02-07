#ifndef GENERIC_SET_H
#define GENERIC_SET_H

#ifndef GENERIC_SET_NO_RAPIDHASH
#define GENERIC_SET_NO_RAPIDHASH false
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if !GENERIC_SET_NO_RAPIDHASH
#include "rapidhash/rapidhash.h"
#endif

#define HASH_NIL 0
#define NODE_COLOR_BLACK 0;
#define NODE_COLOR_RED 1;

#define NODE_NIL                                                               \
  {                                                                            \
      .hash = HASH_NIL,                                                        \
      .left = NULL,                                                            \
      .right = NULL,                                                           \
  };

#define set_type(entry_type)                                                   \
  struct {                                                                     \
    struct node {                                                              \
      struct node *left;                                                       \
      struct node *right;                                                      \
      struct node *parent;                                                     \
      uint64_t hash;                                                           \
      entry_type entry;                                                        \
      uint8_t color;                                                           \
    } *root;                                                                   \
    uint64_t (*hash_fn)(entry_type);                                           \
  }

#define set_init(set, hash_function)                                           \
  do {                                                                         \
    set.root = malloc(sizeof(typeof(*set.root)));                              \
    *set.root = (typeof(*set.root))NODE_NIL;                                   \
    set.hash_fn = hash_function;                                               \
  } while (0);

#define set_free(set)                                                          \
  do {                                                                         \
    typeof(set.root) node = set.root;                                          \
    typeof(set.root) next;                                                     \
    do {                                                                       \
      next = set_next(node);                                                   \
      free(node);                                                              \
    } while ((node = next));                                                   \
  } while (0);

#define set_seq_in_branch(node, f_branch, f_direction)                         \
  ({                                                                           \
    typeof(node) retval = NULL;                                                \
    if (node->hash != HASH_NIL) {                                              \
      retval = node->f_branch;                                                 \
                                                                               \
      if (retval->hash != HASH_NIL) {                                          \
        while (retval->f_direction->hash != HASH_NIL) {                        \
          retval = retval->f_direction;                                        \
        }                                                                      \
      } else {                                                                 \
        retval = NULL;                                                         \
      }                                                                        \
    }                                                                          \
                                                                               \
    retval;                                                                    \
  })

#define set_next_in_branch(node) set_seq_in_branch(node, right, left);
#define set_prev_in_branch(node) set_seq_in_branch(node, left, right);

#define set_seq(node, f_branch, f_direction)                                   \
  ({                                                                           \
    typeof(node) retval = set_seq_in_branch(node, f_branch, f_direction);      \
                                                                               \
    if (retval == NULL) {                                                      \
      uint64_t hash = (node)->hash;                                            \
      typeof(node) retval = (node)->parent;                                    \
      do {                                                                     \
        if (retval->f_direction->hash == hash) {                               \
          break;                                                               \
        }                                                                      \
        hash = retval->hash;                                                   \
      } while ((retval = retval->parent));                                     \
    }                                                                          \
                                                                               \
    retval;                                                                    \
  })

#define set_next(node) set_seq(node, right, left)
#define set_prev(node) set_seq(node, left, right)

#define set_ult(set, f_branch, f_direction)                                    \
  ({                                                                           \
    typeof(set.root) node = set.root;                                          \
    while (true) {                                                             \
      typeof(set.root) next = set_seq(node, f_branch, f_direction);            \
      if (next == NULL) {                                                      \
        break;                                                                 \
      }                                                                        \
      node = next;                                                             \
    }                                                                          \
    node;                                                                      \
  })

#define seq_first(set) seq_ult(node, left, right)
#define seq_last(set) seq_ult(node, right, left)

#define find_node(root, key)                                                   \
  ({                                                                           \
    typeof(root) retval = root;                                                \
    while (retval->hash != HASH_NIL) {                                         \
      if (key > retval->hash) {                                                \
        retval = root->right;                                                  \
      } else if (key < retval->hash) {                                         \
        retval = root->left;                                                   \
      } else {                                                                 \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    retval;                                                                    \
  })

// TODO(2025-02-03, Max Bolotin): Replace malloc with custom allocator
#define set_add(set, entry_var)                                                \
  do {                                                                         \
    printf("Adding %d to set\n", entry_var);                                   \
    uint64_t key = set.hash_fn(entry_var);                                     \
    typeof(set.root) leaf = find_node(set.root, key);                          \
    printf("Found node\n");                                                    \
                                                                               \
    if (leaf->hash != HASH_NIL) {                                              \
      break;                                                                   \
    }                                                                          \
    printf("Found leaf does not NIL hash, setting new leaf data\n");           \
                                                                               \
    typeof(set.root) new_leaves = malloc(sizeof(typeof(*set.root)) * 2);       \
    new_leaves[0] = (typeof(*set.root))NODE_NIL;                               \
    new_leaves[1] = (typeof(*set.root))NODE_NIL;                               \
    new_leaves[0].parent = leaf;                                               \
    new_leaves[1].parent = leaf;                                               \
                                                                               \
    *leaf = (typeof(*set.root)){                                               \
        .hash = key,                                                           \
        .entry = entry_var,                                                    \
        .parent = leaf->parent,                                                \
        .left = &new_leaves[0],                                                \
        .right = &new_leaves[1],                                               \
    };                                                                         \
                                                                               \
    set_rb_insert_fixup(set, leaf);                                            \
  } while (0);

#define set_rb_insert_fixup_dir(set, node, f_branch, f_direction)              \
  do {                                                                         \
    typeof(node) uncle = node->parent->parent->f_branch;                       \
    if (uncle->color == NODE_COLOR_RED) {                                      \
      node->parent->color = NODE_COLOR_BLACK;                                  \
      uncle->color = NODE_COLOR_BLACK;                                         \
      node->parent->parent->color = NODE_COLOR_BLACK;                          \
    } else {                                                                   \
      if (node == node->parent->f_branch) {                                    \
        node = node->parent;                                                   \
        set_rot(set, node, f_branch, f_direction);                             \
      }                                                                        \
      node->parent->color = NODE_COLOR_BLACK;                                  \
      node->parent->parent->color = NODE_COLOR_RED;                            \
      set_rot(set, node->parent, f_direction, f_branch);                       \
    }                                                                          \
  } while (0);

#define set_rb_insert_fixup_left(set, node)                                    \
  set_rb_insert_fixup_dir(set, node, right, left)
#define set_rb_insert_fixup_right(set, node)                                   \
  set_rb_insert_fixup_dir(set, node, left, right)

#define set_rb_insert_fixup(set, node)                                         \
  do {                                                                         \
    typeof(node) n = node;                                                     \
    while (n->parent->color == NODE_COLOR_RED) {                               \
      if (n->parent == n->parent->parent->left) {                              \
        set_rb_insert_fixup_left(set, n);                                      \
      } else {                                                                 \
        set_rb_insert_fixup_right(set, n);                                     \
      }                                                                        \
    }                                                                          \
    set.root->color = NODE_COLOR_RED;                                          \
  } while (0);

#define set_rb_insert(set, node)                                               \
  do {                                                                         \
  } while (0);

#define set_remove(set, entry)                                                 \
  do {                                                                         \
    uint64_t hash = set.hash_fn(entry);                                        \
    typeof(set.root) node = find_node(set.root, hash);                         \
                                                                               \
    if (node->hash != hash) {                                                  \
      break;                                                                   \
    }                                                                          \
                                                                               \
    typeof(set.root) orphan = NULL;                                            \
    if (node->left->hash == HASH_NIL) {                                        \
      orphan = node->right;                                                    \
    } else if (node->right->hash == HASH_NIL) {                                \
      orphan = node->left;                                                     \
    } else {                                                                   \
      typeof(set.root) *next = set_next_in_branch(node);                       \
      assert(next != NULL);                                                    \
      node->hash = next->hash;                                                 \
      node->entry = next->entry;                                               \
      free(next->left);                                                        \
      free(next->right);                                                       \
      *next = (typeof(*set.root))NODE_NIL;                                     \
      break;                                                                   \
    }                                                                          \
                                                                               \
    *node = *orphan;                                                           \
    free(orphan);                                                              \
  } while (0);

#define set_getidx(set, v_idx)                                                 \
  ({                                                                           \
    typeof(set.root) node = set.root;                                          \
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
    typeof(set.root) node = set.root;                                          \
    size_t size = 0;                                                           \
    do {                                                                       \
      size++;                                                                  \
    } while ((node = set_next(node)));                                         \
    size;                                                                      \
  })

#define set_has(set, entry)                                                    \
  ({                                                                           \
    uint64_t hash = set.hash_fn(entry);                                        \
    typeof(set.root) *node = find_node(set.root, key);                         \
    node->hash == key;                                                         \
  })

#define set_rot(set, node, f_branch, f_direction)                              \
  do {                                                                         \
    typeof(node) f_branch = node->f_branch;                                    \
    assert(f_branch->hash != HASH_NIL);                                        \
    node->f_branch = f_branch->f_direction;                                    \
    if (f_branch->f_direction->hash != HASH_NIL) {                             \
      f_branch->f_direction->parent = node;                                    \
    }                                                                          \
    f_branch->parent = node->parent;                                           \
    if (node->parent == NULL) {                                                \
      set.root = f_branch;                                                     \
    } else if (node->hash == node->parent->f_direction->hash) {                \
      node->parent->f_direction = f_branch;                                    \
    } else {                                                                   \
      node->parent->f_branch = f_branch;                                       \
    }                                                                          \
    f_branch->f_direction = node;                                              \
    node->parent = f_branch;                                                   \
  } while (0);

#define set_rot_left(set, node) set_rot(set, node, right, left)
#define set_rot_right(set, node) set_rot(set, node, left, right)

inline uint64_t set_uint32_hash_fn(uint32_t entry) {
  entry = ((entry >> 16) ^ entry) * 0x45d9f3b;
  entry = ((entry >> 16) ^ entry) * 0x45d9f3b;
  entry = (entry >> 16) ^ entry;
  entry |= (1 << 31);
  return entry;
}

inline uint64_t set_uint64_hash_fn(uint64_t entry) {
  entry = (entry ^ (entry >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  entry = (entry ^ (entry >> 27)) * UINT64_C(0x94d049bb133111eb);
  entry = entry ^ (entry >> 31);
  entry |= (1 << 31);
  return entry;
}

#if !GENERIC_SET_NO_RAPIDHASH
inline uint64_t set_string_hash_fn(const char *entry) {
  return rapidhash(entry, strlen(entry));
}
#endif

#endif // !GENERIC_SET_H
