#include <stdint.h>

#include "set.h"
#include "setdebug.h"
#include "trace.h"
#include "unity.h"

uint64_t hash_fn(uint32_t value) { return value; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

typedef map_type(uint32_t, const char *) map_t;

void test_inserts(void) {
  map_t map;
  map_init(map, hash_fn, equals_fn);

  TEST_ASSERT_EQUAL(0, map_size(map));

  map_add(map, 100, "Foo");

  TEST_ASSERT_EQUAL(1, map_size(map));
  TEST_ASSERT_EQUAL(true, map_has(map, 100));

  const char **value = map_get(map, 100);
  TEST_ASSERT_NOT_NULL(value);

  TEST_ASSERT_EQUAL_STRING("Foo", *value);

  map_free(map);
}

void test_deletes(void) {
  map_t map;
  map_init(map, hash_fn, equals_fn);

  TEST_ASSERT_EQUAL(0, map_size(map));

  map_add(map, 1, "foo");
  map_add(map, 2, "bar");
  map_add(map, 3, "baz");
  map_add(map, 4, "lorem");
  map_add(map, 5, "ipsum");
  map_add(map, 6, "dolor");

  TEST_ASSERT_EQUAL(6, map_size(map));
  TEST_ASSERT_EQUAL(2, debug_node_blackheight(map.nodes, map.colors, map.inited,
                                              map.root, true, true));
  TEST_ASSERT_EQUAL(3, map.root);
  TEST_ASSERT_EQUAL(true, map_has(map, 3));
  TEST_ASSERT_NOT_NULL(map_get(map, 3));

  map_remove(map, 3);

  TEST_ASSERT_EQUAL(5, map_size(map));
  TEST_ASSERT_EQUAL(false, map_has(map, 3));
  TEST_ASSERT_NULL(map_get(map, 3));
}
