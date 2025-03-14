#include "set.h"
#include "setdebug.h"
#include "unity.h"
#include <stdint.h>

uint64_t hash_fn(uint32_t value) { return value; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

typedef set_type(uint32_t) set_t;

void setUp(void) {}
void tearDown(void) {}

void test_clone_set(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  TEST_ASSERT_EQUAL(0x00, set.inited[0]);
  TEST_ASSERT_EQUAL(0x00, set.colors[0]);

  set_t clone = set_clone(set);

  TEST_ASSERT_EQUAL(0x00, clone.inited[0]);
  TEST_ASSERT_EQUAL(0x00, clone.colors[0]);
  TEST_ASSERT_EQUAL(set.free_list[0], clone.free_list[0]);
  TEST_ASSERT_EQUAL(set.colors[0], clone.colors[0]);
  TEST_ASSERT_EQUAL(set.inited[0], clone.inited[0]);
}
