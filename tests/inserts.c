#include "set.h"
#include "unity.h"
#include <stdint.h>

uint64_t set_uint32_hash_fn(uint32_t value) { return value + 1; }

typedef set_type(uint32_t) set_uint32_t;

void setUp(void) {}
void tearDown(void) {}

void test_addr_idx_conversions(void) {
  size_t addr = set_addr(0);
  size_t idx = set_idx(addr);
  TEST_ASSERT_EQUAL(0, idx);

  addr = set_addr(1);
  idx = set_idx(addr);
  TEST_ASSERT_EQUAL(1, idx);

  addr = set_addr(1 << 62);
  idx = set_idx(addr);
  TEST_ASSERT_EQUAL((size_t)1 << 62, idx);

  // Idx gets truncated to 0 when converted back and forth
  addr = set_addr(1 << 63);
  idx = set_idx(addr);
  TEST_ASSERT_EQUAL(0, idx);
}

void test_initing_set(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  TEST_ASSERT_EQUAL((size_t)1 << 63, set.root);
  TEST_ASSERT_EQUAL(0, set_idx(set.root));

  size_t root_node_idx = set_idx(set.root);
  TEST_ASSERT_EQUAL(root_node_idx, 0);

  typeof(*set.nodes) root_node = set.nodes[root_node_idx];
  uint8_t root_color = set_read_color(set, root_node_idx);
  TEST_ASSERT_EQUAL(root_node.hash, HASH_NIL);
  TEST_ASSERT_EQUAL(root_node.parent, IDX_NIL);
  TEST_ASSERT_EQUAL(root_node.left, IDX_NIL);
  TEST_ASSERT_EQUAL(root_node.right, IDX_NIL);
  TEST_ASSERT_EQUAL(root_color, 0);

  TEST_ASSERT_EQUAL(set_has(set, 0), false);

  set_free(set);
}

void test_add_first_member(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);
  set_add(set, 2);

  size_t root_node_idx = set_idx(set.root);
  TEST_ASSERT_EQUAL(root_node_idx, 0);

  typeof(*set.nodes) root_node = set.nodes[root_node_idx];
  typeof(*set.entries) root_entry = set.entries[root_node_idx];
  bool root_color = set_read_color(set, root_node_idx);
  TEST_ASSERT_EQUAL(root_node.hash, set_uint32_hash_fn(2));
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
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_add(set, 6);

  size_t root_node_idx = set_idx(set.root);
  TEST_ASSERT_EQUAL(root_node_idx, 0);

  typeof(*set.nodes) root_node = set.nodes[root_node_idx];
  typeof(*set.entries) root_entry = set.entries[root_node_idx];
  bool root_color = set_read_color(set, root_node_idx);
  TEST_ASSERT_EQUAL(root_node.parent, IDX_NIL);
  TEST_ASSERT_EQUAL(root_entry, 2);
  TEST_ASSERT_EQUAL(root_color, 0);

  size_t left_idx = set_idx(root_node.left);
  TEST_ASSERT_NOT_EQUAL(left_idx, 0);

  size_t right_idx = set_idx(root_node.right);
  TEST_ASSERT_NOT_EQUAL(right_idx, 0);

  typeof(*set.nodes) left = set.nodes[left_idx];

  TEST_ASSERT_EQUAL(left.hash, HASH_NIL);
  TEST_ASSERT_EQUAL(left.left, 0);
  TEST_ASSERT_EQUAL(left.right, 0);

  typeof(*set.nodes) right = set.nodes[right_idx];
  typeof(*set.entries) right_entry = set.entries[right_idx];
  bool right_color = set_read_color(set, right_idx);

  TEST_ASSERT_EQUAL(right.hash, set_uint32_hash_fn(6));
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
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_add(set, 1);

  size_t root_node_idx = set_idx(set.root);
  TEST_ASSERT_EQUAL(root_node_idx, 0);

  typeof(*set.nodes) root_node = set.nodes[root_node_idx];
  typeof(*set.entries) root_entry = set.entries[root_node_idx];
  bool root_color = set_read_color(set, root_node_idx);
  TEST_ASSERT_EQUAL(root_node.parent, IDX_NIL);
  TEST_ASSERT_EQUAL(root_entry, 2);
  TEST_ASSERT_EQUAL(root_color, 0);

  size_t left_idx = set_idx(root_node.left);
  TEST_ASSERT_NOT_EQUAL(left_idx, 0);

  size_t right_idx = set_idx(root_node.right);
  TEST_ASSERT_NOT_EQUAL(right_idx, 0);

  typeof(*set.nodes) left = set.nodes[left_idx];
  typeof(*set.entries) left_entry = set.entries[left_idx];
  bool left_color = set_read_color(set, left_idx);
  TEST_ASSERT_EQUAL(left.hash, set_uint32_hash_fn(1));
  TEST_ASSERT_NOT_EQUAL(set_idx(left.left), 0);
  TEST_ASSERT_NOT_EQUAL(set_idx(left.right), 0);
  TEST_ASSERT_EQUAL(left_entry, 1);
  TEST_ASSERT_EQUAL(left_color, NODE_COLOR_RED);
  TEST_ASSERT_EQUAL(left.parent, set_addr(0));

  typeof(*set.nodes) right = set.nodes[right_idx];
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
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);

  size_t root_node_idx = set_idx(set.root);

  typeof(*set.nodes) root_node = set.nodes[root_node_idx];
  typeof(*set.entries) root_entry = set.entries[root_node_idx];
  TEST_ASSERT_EQUAL(root_entry, 3);

  size_t left_idx = set_idx(root_node.left);
  size_t right_idx = set_idx(root_node.right);

  typeof(*set.nodes) left = set.nodes[left_idx];
  typeof(*set.nodes) right = set.nodes[right_idx];

  typeof(*set.entries) left_entry = set.entries[left_idx];
  typeof(*set.entries) right_entry = set.entries[right_idx];

  TEST_ASSERT_EQUAL(left_entry, 2);
  TEST_ASSERT_EQUAL(left.parent, set_addr(root_node_idx));
  TEST_ASSERT_EQUAL(set_get_node(set, left.left)->hash, HASH_NIL);
  TEST_ASSERT_EQUAL(set_get_node(set, left.right)->hash, HASH_NIL);

  TEST_ASSERT_EQUAL(right_entry, 4);
  TEST_ASSERT_EQUAL(right.parent, set_addr(root_node_idx));
  TEST_ASSERT_EQUAL(set_get_node(set, right.left)->hash, HASH_NIL);
  TEST_ASSERT_EQUAL(set_get_node(set, right.right)->hash, HASH_NIL);

  bool has_two = set_has(set, 2);
  if (has_two) {
  }

  // TEST_ASSERT_EQUAL(set_has(set, 2), true);
  // TEST_ASSERT_EQUAL(set_has(set, 3), true);
  // TEST_ASSERT_EQUAL(set_has(set, 4), true);
  // TEST_ASSERT_EQUAL(set_has(set, 5), false);
  // TEST_ASSERT_EQUAL(set_has(set, 6), false);
  // TEST_ASSERT_EQUAL(set_has(set, 7), false);

  // set_free(set);
}

void test_add_third_member_triangle(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_add(set, 5);
  set_add(set, 4);

  size_t root_node_idx = set_idx(set.root);

  typeof(*set.nodes) root_node = set.nodes[root_node_idx];
  typeof(*set.entries) root_entry = set.entries[root_node_idx];
  bool root_color = set_read_color(set, root_node_idx);
  TEST_ASSERT_EQUAL(root_entry, 4);
  TEST_ASSERT_EQUAL(root_color, NODE_COLOR_BLACK);

  size_t left_idx = set_idx(root_node.left);
  size_t right_idx = set_idx(root_node.right);

  typeof(*set.nodes) left = set.nodes[left_idx];
  typeof(*set.nodes) right = set.nodes[right_idx];

  typeof(*set.entries) left_entry = set.entries[left_idx];
  typeof(*set.entries) right_entry = set.entries[right_idx];

  bool left_color = set_read_color(set, left_idx);
  bool right_color = set_read_color(set, right_idx);

  TEST_ASSERT_EQUAL(left_entry, 2);
  TEST_ASSERT_EQUAL(left.parent, set_addr(root_node_idx));
  TEST_ASSERT_EQUAL(set_get_node(set, left.left)->hash, HASH_NIL);
  TEST_ASSERT_EQUAL(set_get_node(set, left.right)->hash, HASH_NIL);
  TEST_ASSERT_EQUAL(left_color, NODE_COLOR_RED);

  TEST_ASSERT_EQUAL(right_entry, 5);
  TEST_ASSERT_EQUAL(right.parent, set_addr(root_node_idx));
  TEST_ASSERT_EQUAL(set_get_node(set, right.left)->hash, HASH_NIL);
  TEST_ASSERT_EQUAL(set_get_node(set, right.right)->hash, HASH_NIL);
  TEST_ASSERT_EQUAL(right_color, NODE_COLOR_RED);
}

void test_insert_sizeidentity(void) {
  set_uint32_t set;

  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_add(set, 6);
  set_add(set, 1);
  set_add(set, 10);
  set_add(set, 6); // Duplicate of second add

  size_t size = set_size(set);
  TEST_ASSERT_EQUAL(4, size);

  set_free(set);
}
