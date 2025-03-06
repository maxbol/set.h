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
