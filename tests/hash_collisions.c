#include <stdint.h>

#include "set.h"
#include "setdebug.h"
#include "unity.h"

// Corrupt hashing function - always return 1 as hash
uint64_t hash_fn(uint32_t value) { return 1; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

typedef set_type(uint32_t) set_t;

void setUp(void) {}
void tearDown(void) {}

void test_collision_on_add(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 1);
  set_add(set, 2);
  set_add(set, 2);

  TEST_ASSERT_EQUAL(1, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));
  TEST_ASSERT_EQUAL(2, set_size(set));
}

void test_self_balancing_single_hash(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 1);
  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);
  set_add(set, 5);
  set_add(set, 6);
  set_add(set, 7);
  set_add(set, 8);
  set_add(set, 9);
  set_add(set, 10);

  TEST_ASSERT_EQUAL(10, set_size(set));
  TEST_ASSERT_EQUAL(3, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));
}

void test_correctly_identifies_has(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 1);
  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);
  set_add(set, 5);

  TEST_ASSERT_EQUAL(true, set_has(set, 1));
  TEST_ASSERT_EQUAL(true, set_has(set, 2));
  TEST_ASSERT_EQUAL(true, set_has(set, 3));
  TEST_ASSERT_EQUAL(true, set_has(set, 4));
  TEST_ASSERT_EQUAL(true, set_has(set, 5));
  TEST_ASSERT_EQUAL(false, set_has(set, 6));
}

void test_remove_hash_double(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, 1);
  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);
  set_add(set, 5);

  set_remove(set, 3);

  TEST_ASSERT_EQUAL(false, set_has(set, 3));
  TEST_ASSERT_EQUAL(true, set_has(set, 1));
  TEST_ASSERT_EQUAL(true, set_has(set, 2));
  TEST_ASSERT_EQUAL(true, set_has(set, 4));
  TEST_ASSERT_EQUAL(true, set_has(set, 5));
}
