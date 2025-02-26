#include "set.h"
#include "setdebug.h"
#include "unity.h"
#include <stdint.h>

uint64_t set_uint32_hash_fn(uint32_t value) { return value; }

typedef set_type(uint32_t) set_uint32_t;

void setUp(void) {}
void tearDown(void) {}

void test_delete_only_member(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_remove(set, 2);

  TEST_ASSERT_EQUAL(set_size(set), 0);
  TEST_ASSERT_EQUAL(set_is_inited(set, set.root), 0);
}

void test_delete_root_with_two_children(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 1);
  set_add(set, 2);
  set_add(set, 3);

  TEST_ASSERT_EQUAL(set_get_entry(set, set.root), 2);

  set_remove(set, 2);

  TEST_ASSERT_EQUAL(set_size(set), 2);
  TEST_ASSERT_EQUAL(set_node_blackheight(set.nodes, set.colors, set.inited,
                                         set.root, true, true),
                    1);
}

void test_delete_red_parent_of_two_black_childless(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  for (uint32_t i = 1; i == 6; i++) {
    set_add(set, i);
  }

  set_remove(set, 6);
  set_remove(set, 4);

  TEST_ASSERT_EQUAL(set_size(set), 4);
  TEST_ASSERT_EQUAL(set_node_blackheight(set.nodes, set.colors, set.inited,
                                         set.root, true, true),
                    2);
}
