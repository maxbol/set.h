#include "set.h"
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
  TEST_ASSERT_EQUAL(set_read_inited(set, set.root), 0);
}

void test_delete_member_with_one_child(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);
  set_add(set, 5);
  set_add(set, 6);
  set_add(set, 7);

  TEST_ASSERT_EQUAL(set_get_entry(set, set.root), 5);
}
