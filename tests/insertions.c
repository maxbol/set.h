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
  size_t addr = set_addr(0);
  size_t idx = set_idx(addr);
  TEST_ASSERT_EQUAL(0, idx);

  addr = set_addr(1);
  idx = set_idx(addr);
  TEST_ASSERT_EQUAL(1, idx);
}

void test_initing_set(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  TEST_ASSERT_EQUAL(1, set.root);

  typeof(*set.nodes) root_node = *set_get_node(set, set.root);
  uint8_t root_color = set_is_red(set, set.root);
  TEST_ASSERT_EQUAL(root_node.hash, HASH_NIL);
  TEST_ASSERT_EQUAL(root_node.parent, IDX_NIL);
  TEST_ASSERT_EQUAL(root_node.left, IDX_NIL);
  TEST_ASSERT_EQUAL(root_node.right, IDX_NIL);
  TEST_ASSERT_EQUAL(root_color, 0);

  TEST_ASSERT_EQUAL(set_has(set, 0), false);

  set_free(set);
}

void test_add_first_member(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);
  set_add(set, 2);

  TEST_ASSERT_EQUAL(set.root, 1);

  typeof(*set.nodes) root_node = *set_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = set_is_red(set, set.root);
  TEST_ASSERT_EQUAL(root_node.hash, hash_fn(2));
  // TODO(2025-02-09, Max Bolotin): Parent is non-nil here, and shouldn't be.
  // Fix it!!
  TEST_ASSERT_EQUAL(root_node.parent, IDX_NIL);
  TEST_ASSERT_EQUAL(root_entry, 2);
  TEST_ASSERT_EQUAL(root_color, 0);

  size_t left_idx = set_idx(root_node.left);
  TEST_ASSERT_NOT_EQUAL(left_idx, 0);

  size_t right_idx = set_idx(root_node.right);
  TEST_ASSERT_NOT_EQUAL(right_idx, 0);

  TEST_ASSERT_EQUAL(set_has(set, 2), true);
  TEST_ASSERT_EQUAL(set_has(set, 3), false);

  set_free(set);
}

void test_add_second_larger_member(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 6);

  TEST_ASSERT_EQUAL(set.root, 1);

  typeof(*set.nodes) root_node = *set_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = set_is_red(set, set.root);
  TEST_ASSERT_EQUAL(root_node.parent, IDX_NIL);
  TEST_ASSERT_EQUAL(root_entry, 2);
  TEST_ASSERT_EQUAL(root_color, 0);

  TEST_ASSERT_NOT_EQUAL(root_node.left, 0);
  TEST_ASSERT_NOT_EQUAL(root_node.right, 0);

  typeof(*set.nodes) left = *set_get_node(set, root_node.left);

  TEST_ASSERT_EQUAL(left.hash, HASH_NIL);
  TEST_ASSERT_EQUAL(left.left, 0);
  TEST_ASSERT_EQUAL(left.right, 0);

  typeof(*set.nodes) right = *set_get_node(set, root_node.right);
  typeof(*set.entries) right_entry = set_get_entry(set, root_node.right);
  bool right_color = set_is_red(set, root_node.right);

  TEST_ASSERT_EQUAL(right.hash, hash_fn(6));
  TEST_ASSERT_NOT_EQUAL(set_idx(right.left), 0);
  TEST_ASSERT_NOT_EQUAL(set_idx(right.right), 0);
  TEST_ASSERT_EQUAL(right_entry, 6);
  TEST_ASSERT_EQUAL(right_color, NODE_COLOR_RED);
  TEST_ASSERT_EQUAL(right.parent, set_addr(0));

  TEST_ASSERT_EQUAL(set_has(set, 2), true);
  TEST_ASSERT_EQUAL(set_has(set, 6), true);
  TEST_ASSERT_EQUAL(set_has(set, 3), false);
  TEST_ASSERT_EQUAL(set_has(set, 7), false);

  set_free(set);
}

void test_add_second_smaller_member(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 1);

  TEST_ASSERT_EQUAL(set.root, set_addr(0));
  typeof(*set.nodes) root_node = *set_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = set_is_red(set, set.root);
  TEST_ASSERT_EQUAL(root_node.parent, IDX_NIL);
  TEST_ASSERT_EQUAL(root_entry, 2);
  TEST_ASSERT_EQUAL(root_color, 0);

  TEST_ASSERT_NOT_EQUAL(root_node.left, 0);
  TEST_ASSERT_NOT_EQUAL(root_node.right, 0);

  typeof(*set.nodes) left = *set_get_node(set, root_node.left);
  typeof(*set.entries) left_entry = set_get_entry(set, root_node.left);
  bool left_color = set_is_red(set, root_node.left);
  TEST_ASSERT_EQUAL(left.hash, hash_fn(1));
  TEST_ASSERT_NOT_EQUAL(left.left, 0);
  TEST_ASSERT_NOT_EQUAL(left.right, 0);
  TEST_ASSERT_EQUAL(left_entry, 1);
  TEST_ASSERT_EQUAL(left_color, NODE_COLOR_RED);
  TEST_ASSERT_EQUAL(left.parent, set_addr(0));

  typeof(*set.nodes) right = *set_get_node(set, root_node.right);
  TEST_ASSERT_EQUAL(right.hash, HASH_NIL);
  TEST_ASSERT_EQUAL(right.left, 0);
  TEST_ASSERT_EQUAL(right.right, 0);

  TEST_ASSERT_EQUAL(set_has(set, 2), true);
  TEST_ASSERT_EQUAL(set_has(set, 1), true);
  TEST_ASSERT_EQUAL(set_has(set, 3), false);
  TEST_ASSERT_EQUAL(set_has(set, 4), false);

  set_free(set);
}

void test_add_third_member_line(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);

  typeof(*set.nodes) root_node = *set_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  TEST_ASSERT_EQUAL(root_entry, 3);

  TEST_ASSERT_NOT_EQUAL(root_node.left, 0);
  TEST_ASSERT_NOT_EQUAL(root_node.right, 0);

  typeof(*set.nodes) left = *set_get_node(set, root_node.left);
  typeof(*set.nodes) right = *set_get_node(set, root_node.right);

  typeof(*set.entries) left_entry = set_get_entry(set, root_node.left);
  typeof(*set.entries) right_entry = set_get_entry(set, root_node.right);

  TEST_ASSERT_EQUAL(left_entry, 2);
  TEST_ASSERT_EQUAL(left.parent, set.root);
  TEST_ASSERT_EQUAL(set_is_inited(set, left.left), false);
  TEST_ASSERT_EQUAL(set_is_inited(set, left.right), false);

  TEST_ASSERT_EQUAL(right_entry, 4);
  TEST_ASSERT_EQUAL(right.parent, set.root);
  TEST_ASSERT_EQUAL(set_is_inited(set, right.left), false);
  TEST_ASSERT_EQUAL(set_is_inited(set, right.right), false);

  TEST_ASSERT_EQUAL(set_has(set, 2), true);
  TEST_ASSERT_EQUAL(set_has(set, 3), true);
  TEST_ASSERT_EQUAL(set_has(set, 4), true);
  TEST_ASSERT_EQUAL(set_has(set, 5), false);
  TEST_ASSERT_EQUAL(set_has(set, 6), false);
  TEST_ASSERT_EQUAL(set_has(set, 7), false);

  TEST_ASSERT_EQUAL(set_node_blackheight(set.nodes, set.colors, set.inited,
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

  typeof(*set.nodes) root_node = *set_get_node(set, set.root);
  typeof(*set.entries) root_entry = set_get_entry(set, set.root);
  bool root_color = set_is_red(set, set.root);
  TEST_ASSERT_EQUAL(root_entry, 4);
  TEST_ASSERT_EQUAL(root_color, NODE_COLOR_BLACK);

  TEST_ASSERT_NOT_EQUAL(root_node.left, 0);
  TEST_ASSERT_NOT_EQUAL(root_node.right, 0);

  typeof(*set.nodes) left = *set_get_node(set, root_node.left);
  typeof(*set.nodes) right = *set_get_node(set, root_node.right);

  typeof(*set.entries) left_entry = set_get_entry(set, root_node.left);
  typeof(*set.entries) right_entry = set_get_entry(set, root_node.right);

  bool left_color = set_is_red(set, root_node.left);
  bool right_color = set_is_red(set, root_node.right);

  TEST_ASSERT_EQUAL(left_entry, 2);
  TEST_ASSERT_EQUAL(left.parent, set.root);
  TEST_ASSERT_EQUAL(set_is_inited(set, left.left), false);
  TEST_ASSERT_EQUAL(set_is_inited(set, left.right), false);
  TEST_ASSERT_EQUAL(left_color, NODE_COLOR_RED);

  TEST_ASSERT_EQUAL(right_entry, 5);
  TEST_ASSERT_EQUAL(right.parent, set.root);
  TEST_ASSERT_EQUAL(set_is_inited(set, right.left), false);
  TEST_ASSERT_EQUAL(set_is_inited(set, right.right), false);
  TEST_ASSERT_EQUAL(right_color, NODE_COLOR_RED);

  TEST_ASSERT_EQUAL(set_node_blackheight(set.nodes, set.colors, set.inited,
                                         set.root, true, true),
                    1);

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

  TEST_ASSERT_EQUAL(set_node_blackheight(set.nodes, set.colors, set.inited,
                                         set.root, true, true),
                    2);

  TEST_ASSERT_EQUAL(set_get_entry(set, set.root), 5);

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

  TEST_ASSERT_EQUAL(set_node_blackheight(set.nodes, set.colors, set.inited,
                                         set.root, true, true),
                    2);

  size_t size = set_size(set);
  TEST_ASSERT_EQUAL(4, size);

  set_free(set);
}
