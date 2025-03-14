#include <stdint.h>

#include "set.h"
#include "setdebug.h"
#include "trace.h"
#include "unity.h"

uint64_t hash_fn(uint32_t value) { return value; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

typedef set_type(uint32_t) set_t;

void setUp(void) {}
void tearDown(void) {}

void test_addr_idx_conversions(void) {
  tree_addr_t addr = tree_addr(0);
  tree_idx_t idx = tree_idx(addr);
  TEST_ASSERT_EQUAL(0, idx);

  addr = tree_addr(1);
  idx = tree_idx(addr);
  TEST_ASSERT_EQUAL(1, idx);
}

void test_initing_set(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  TEST_ASSERT_EQUAL(tree_addr(0), set.root);

  typeof(*set.nodes) root_node = *tree_get_node(set, set.root);
  uint8_t root_color = tree_is_red(set, set.root);
  TEST_ASSERT_EQUAL(HASH_NIL, root_node.hash);
  TEST_ASSERT_EQUAL(IDX_NIL, root_node.parent);
  TEST_ASSERT_EQUAL(IDX_NIL, root_node.left);
  TEST_ASSERT_EQUAL(IDX_NIL, root_node.right);
  TEST_ASSERT_EQUAL(0, root_color);

  TEST_ASSERT_EQUAL(false, set_has(set, 0));

  set_free(set);
}

void test_add_first_member(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);
  set_add(set, 2);

  TEST_ASSERT_EQUAL(tree_addr(0), set.root);

  typeof(*set.nodes) root_node = *tree_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = tree_is_red(set, set.root);
  TEST_ASSERT_EQUAL(hash_fn(2), root_node.hash);
  TEST_ASSERT_EQUAL(IDX_NIL, root_node.parent);
  TEST_ASSERT_EQUAL(2, root_entry);
  TEST_ASSERT_EQUAL(0, root_color);

  tree_idx_t left_idx = tree_idx(root_node.left);
  TEST_ASSERT_NOT_EQUAL(0, left_idx);

  tree_idx_t right_idx = tree_idx(root_node.right);
  TEST_ASSERT_NOT_EQUAL(0, right_idx);

  TEST_ASSERT_EQUAL(true, set_has(set, 2));
  TEST_ASSERT_EQUAL(false, set_has(set, 3));

  set_free(set);
}

void test_add_second_larger_member(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 6);

  TEST_ASSERT_EQUAL(tree_addr(0), set.root);

  typeof(*set.nodes) root_node = *tree_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = tree_is_red(set, set.root);
  TEST_ASSERT_EQUAL(IDX_NIL, root_node.parent);
  TEST_ASSERT_EQUAL(2, root_entry);
  TEST_ASSERT_EQUAL(0, root_color);

  TEST_ASSERT_NOT_EQUAL(0, root_node.left);
  TEST_ASSERT_NOT_EQUAL(0, root_node.right);

  typeof(*set.nodes) left = *tree_get_node(set, root_node.left);

  TEST_ASSERT_EQUAL(HASH_NIL, left.hash);
  TEST_ASSERT_EQUAL(0, left.left);
  TEST_ASSERT_EQUAL(0, left.right);

  typeof(*set.nodes) right = *tree_get_node(set, root_node.right);
  typeof(*set.entries) right_entry = set_get_entry(set, root_node.right);
  bool right_color = tree_is_red(set, root_node.right);

  TEST_ASSERT_EQUAL(hash_fn(6), right.hash);
  TEST_ASSERT_NOT_EQUAL(0, tree_idx(right.left));
  TEST_ASSERT_NOT_EQUAL(0, tree_idx(right.right));
  TEST_ASSERT_EQUAL(6, right_entry);
  TEST_ASSERT_EQUAL(NODE_COLOR_RED, right_color);
  TEST_ASSERT_EQUAL(tree_addr(0), right.parent);

  TEST_ASSERT_EQUAL(true, set_has(set, 2));
  TEST_ASSERT_EQUAL(true, set_has(set, 6));
  TEST_ASSERT_EQUAL(false, set_has(set, 3));
  TEST_ASSERT_EQUAL(false, set_has(set, 7));

  set_free(set);
}

void test_add_second_smaller_member(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 1);

  TEST_ASSERT_EQUAL(tree_addr(0), set.root);
  typeof(*set.nodes) root_node = *tree_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = tree_is_red(set, set.root);
  TEST_ASSERT_EQUAL(IDX_NIL, root_node.parent);
  TEST_ASSERT_EQUAL(2, root_entry);
  TEST_ASSERT_EQUAL(0, root_color);

  TEST_ASSERT_NOT_EQUAL(0, root_node.left);
  TEST_ASSERT_NOT_EQUAL(0, root_node.right);

  typeof(*set.nodes) left = *tree_get_node(set, root_node.left);
  typeof(*set.entries) left_entry = set_get_entry(set, root_node.left);
  bool left_color = tree_is_red(set, root_node.left);
  TEST_ASSERT_EQUAL(hash_fn(1), left.hash);
  TEST_ASSERT_NOT_EQUAL(0, left.left);
  TEST_ASSERT_NOT_EQUAL(0, left.right);
  TEST_ASSERT_EQUAL(1, left_entry);
  TEST_ASSERT_EQUAL(NODE_COLOR_RED, left_color);
  TEST_ASSERT_EQUAL(tree_addr(0), left.parent);

  typeof(*set.nodes) right = *tree_get_node(set, root_node.right);
  TEST_ASSERT_EQUAL(HASH_NIL, right.hash);
  TEST_ASSERT_EQUAL(0, right.left);
  TEST_ASSERT_EQUAL(0, right.right);

  TEST_ASSERT_EQUAL(true, set_has(set, 2));
  TEST_ASSERT_EQUAL(true, set_has(set, 1));
  TEST_ASSERT_EQUAL(false, set_has(set, 3));
  TEST_ASSERT_EQUAL(false, set_has(set, 4));

  set_free(set);
}

void test_add_third_member_line(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);

  typeof(*set.nodes) root_node = *tree_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  TEST_ASSERT_EQUAL(3, root_entry);

  TEST_ASSERT_NOT_EQUAL(0, root_node.left);
  TEST_ASSERT_NOT_EQUAL(0, root_node.right);

  typeof(*set.nodes) left = *tree_get_node(set, root_node.left);
  typeof(*set.nodes) right = *tree_get_node(set, root_node.right);

  typeof(*set.entries) left_entry = set_get_entry(set, root_node.left);
  typeof(*set.entries) right_entry = set_get_entry(set, root_node.right);

  TEST_ASSERT_EQUAL(2, left_entry);
  TEST_ASSERT_EQUAL(left.parent, set.root);
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, left.left));
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, left.right));

  TEST_ASSERT_EQUAL(4, right_entry);
  TEST_ASSERT_EQUAL(right.parent, set.root);
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, right.left));
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, right.right));

  TEST_ASSERT_EQUAL(true, set_has(set, 2));
  TEST_ASSERT_EQUAL(true, set_has(set, 3));
  TEST_ASSERT_EQUAL(true, set_has(set, 4));
  TEST_ASSERT_EQUAL(false, set_has(set, 5));
  TEST_ASSERT_EQUAL(false, set_has(set, 6));
  TEST_ASSERT_EQUAL(false, set_has(set, 7));

  TEST_ASSERT_EQUAL(debug_node_blackheight(set.nodes, set.colors, set.inited,
                                           set.root, true, true),
                    1);

  set_free(set);
}

void test_add_third_member_triangle(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 5);
  set_add(set, 4);

  typeof(*set.nodes) root_node = *tree_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = tree_is_red(set, set.root);
  TEST_ASSERT_EQUAL(root_entry, 4);
  TEST_ASSERT_EQUAL(root_color, NODE_COLOR_BLACK);

  TEST_ASSERT_NOT_EQUAL(root_node.left, 0);
  TEST_ASSERT_NOT_EQUAL(root_node.right, 0);

  typeof(*set.nodes) left = *tree_get_node(set, root_node.left);
  typeof(*set.nodes) right = *tree_get_node(set, root_node.right);

  typeof(*set.entries) left_entry = set_get_entry(set, root_node.left);
  typeof(*set.entries) right_entry = set_get_entry(set, root_node.right);

  bool left_color = tree_is_red(set, root_node.left);
  bool right_color = tree_is_red(set, root_node.right);

  TEST_ASSERT_EQUAL(2, left_entry);
  TEST_ASSERT_EQUAL(set.root, left.parent);
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, left.left));
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, left.right));
  TEST_ASSERT_EQUAL(NODE_COLOR_RED, left_color);

  TEST_ASSERT_EQUAL(5, right_entry);
  TEST_ASSERT_EQUAL(right.parent, set.root);
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, right.left));
  TEST_ASSERT_EQUAL(false, tree_is_inited(set, right.right));
  TEST_ASSERT_EQUAL(NODE_COLOR_RED, right_color);

  TEST_ASSERT_EQUAL(1, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));

  set_free(set);
}

void test_larger_range_add(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);
  set_add(set, 5);
  set_add(set, 6);
  set_add(set, 7);
  set_add(set, 8);
  set_add(set, 9);

  TEST_ASSERT_EQUAL(2, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));

  TEST_ASSERT_EQUAL(5, set_get_entry(set, set.root));

  set_free(set);
}

void test_insert_sizeidentity(void) {
  set_t set;

  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 6);
  set_add(set, 1);
  set_add(set, 10);
  set_add(set, 6);

  TEST_ASSERT_EQUAL(2, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));

  size_t size = set_size(set);
  TEST_ASSERT_EQUAL(4, size);

  set_free(set);
}
