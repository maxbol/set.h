#include "rapidhash/rapidhash.h"
#include "set.h"
#include "setdebug.h"
#include "trace.h"
#include "unity.h"

uint64_t hash_fn(const char *value) { return rapidhash(value, strlen(value)); }
bool equals_fn(const char *a, const char *b) { return strcmp(a, b) == 0; }

typedef set_type(const char *) set_t;

void test_has_stringval(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, "foo");
  TEST_ASSERT_EQUAL(true, set_has(set, "foo"));
  TEST_ASSERT_EQUAL(false, set_has(set, "bar"));
}

void test_correct_setsize_with_duplicates(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, "foo");
  set_add(set, "bar");
  set_add(set, "baz");
  set_add(set, "lorem");
  set_add(set, "ipsum");
  set_add(set, "dolor");

  set_add(set, "bar");
  set_add(set, "ipsum");

  TEST_ASSERT_EQUAL(6, set_size(set));
}

void test_set_remove_string(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  set_add(set, "foo");
  set_add(set, "bar");
  set_add(set, "baz");

  set_remove(set, "bar");

  TEST_ASSERT_EQUAL(2, set_size(set));
  TEST_ASSERT_EQUAL(false, set_has(set, "bar"));
  TEST_ASSERT_EQUAL(true, set_has(set, "foo"));
  TEST_ASSERT_EQUAL(true, set_has(set, "baz"));
  TEST_ASSERT_EQUAL(1, set_node_blackheight(set.nodes, set.colors, set.inited,
                                            set.root, true, true));
}
