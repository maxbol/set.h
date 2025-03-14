#include "set.h"
#include "setdebug.h"
#include "unity.h"
#include <stdint.h>

uint64_t hash_fn(uint32_t value) { return value; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

typedef set_type(uint32_t) set_t;

void setUp(void) {}
void tearDown(void) {}

void test_delete_only_member(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_remove(set, 2);

  TEST_ASSERT_EQUAL(0, set_size(set));
  TEST_ASSERT_EQUAL(0, tree_is_inited(set, set.root));
}

void test_delete_root_with_two_children(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 1);
  set_add(set, 2);
  set_add(set, 3);

  TEST_ASSERT_EQUAL(set_get_entry(set, set.root), 2);

  set_remove(set, 2);

  TEST_ASSERT_EQUAL(2, set_size(set));
  TEST_ASSERT_EQUAL(1, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));
}

void test_delete_red_parent_of_two_black_childless(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  for (uint32_t i = 1; i <= 6; i++) {
    set_add(set, i);
  }

  set_remove(set, 6);
  set_remove(set, 4);

  TEST_ASSERT_EQUAL(4, set_size(set));
  TEST_ASSERT_EQUAL(2, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));
}

void test_deleted_entry_returned_to_free_list(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);

  tree_addr_t old_start = set.free_list_start;

  tree_addr_t addr = tree_find_node(set, 3);
  TEST_ASSERT_EQUAL(true, tree_is_inited(set, addr));

  set_remove(set, 3);

  TEST_ASSERT_EQUAL(false, tree_is_inited(set, addr));
  TEST_ASSERT_NOT_EQUAL(0, set.free_list[tree_idx(addr)]);

  TEST_ASSERT_EQUAL(addr, set.free_list_start);
  TEST_ASSERT_EQUAL(old_start, set.free_list[tree_idx(set.free_list_start)]);
}
