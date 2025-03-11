#include "set.h"
#include "setdebug.h"
#include "trace.h"
#include "unity.h"
#include <stdint.h>

uint64_t hash_fn(uint32_t value) { return value; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

typedef set_type(uint32_t) set_t;

void setUp(void) {}
void tearDown(void) {}

void test_expand_buffer(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  for (int i = 0; i < ALLOC_CHUNK; i++) {
    set_add(set, i);
  }

  // Every set_add() allocs two new leaf nodes, so the amount of needed space
  // for ALLOC_CHUNK no of calls to set_add() is ALLOC_CHUNK * 2. The call to
  // add 255 triggers a realloc, and so does the call to add 511. Everytime a
  // realloc happens, capacity is doubled.
  TEST_ASSERT_EQUAL(ALLOC_CHUNK * 2 * 2, set.capacity);

  for (int i = 0; i < ALLOC_CHUNK; i++) {
    TEST_ASSERT_EQUAL(true, set_has(set, i));
    TEST_ASSERT_EQUAL(false, set_has(set, i + ALLOC_CHUNK));
  }

  size_t blackheight = debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true);
  TEST_ASSERT_EQUAL(8, blackheight);

  int removed_num = 0;
  do {
    TEST_ASSERT_EQUAL(ALLOC_CHUNK - removed_num, set_size(set));
    size_t root = set.root;
    uint32_t root_val = set_get_entry(set, root);
    TEST_ASSERT_EQUAL(true, set_has(set, root_val));
    set_remove(set, root_val);
    TEST_ASSERT_EQUAL(false, set_has(set, root_val));
    removed_num++;
  } while (tree_is_inited(set, set.root));
}

void test_free_prevents_realloc(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  for (int i = 0; i < 255; i++) {
    set_add(set, i);
  }

  TEST_ASSERT_EQUAL(ALLOC_CHUNK, set.capacity);

  for (int i = 0; i < 255; i += 2) {
    set_remove(set, i);
  }

  set_add(set, 255);

  TEST_ASSERT_EQUAL(ALLOC_CHUNK, set.capacity);
}

void test_flags_maintained_after_realloc(void) {
  set_t set;
  set_init(set, hash_fn, equals_fn);

  for (int i = 0; i < 255; i++) {
    set_add(set, i);
  }

  // One more node alloc still doesn't push us over the limit
  set_alloc_new_node(set);

  uint8_t old_colors[64];
  uint8_t old_inited[64];

  for (int i = 0; i < 64; i++) {
    old_colors[i] = set.colors[i];
    old_inited[i] = set.inited[i];
  }

  TEST_ASSERT_EQUAL(ALLOC_CHUNK, set.capacity);

  // This node alloc should trigger a realloc
  set_alloc_new_node(set);

  TEST_ASSERT_EQUAL(ALLOC_CHUNK * 2, set.capacity);

  for (int i = 0; i < 64; i++) {
    TEST_ASSERT_EQUAL(old_colors[i], set.colors[i]);
    TEST_ASSERT_EQUAL(old_inited[i], set.inited[i]);
  }
}

void test_ridiculous_size(void) {
  enable_tracing();

  set_t set;
  set_init(set, hash_fn, equals_fn);

  for (uint32_t i = 0; i < 1024000; i++) {
    set_add(set, i);
  }
  flush_trace_hist();

  TEST_ASSERT_EQUAL(19,
                    debug_node_blackheight(set.nodes, set.colors, set.inited,
                                           set.root, true, true));
}
