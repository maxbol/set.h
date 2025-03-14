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

void test_ridiculous_size(void) {
  enable_tracing();

  set_t set;
  set_init(set, hash_fn, equals_fn);

  struct timespec before, after;
  clock_gettime(CLOCK_MONOTONIC_RAW, &before);

  for (uint32_t i = 0; i < 1024000; i++) {
    set_add(set, i);
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &after);
  uint64_t before_ms = timespec_ms(before);
  uint64_t after_ms = timespec_ms(after);
  printf("Total time: %lld ms\n", after_ms - before_ms);
  flush_trace_hist();

  TEST_ASSERT_EQUAL(19,
                    debug_node_blackheight(set.nodes, set.colors, set.inited,
                                           set.root, true, true));
}
void test_remove_speed(void) {

  set_t set;
  set_init(set, hash_fn, equals_fn);

  struct timespec before, after;
  clock_gettime(CLOCK_MONOTONIC_RAW, &before);

  for (uint32_t i = 0; i < 1024000; i++) {
    set_add(set, i);
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &after);
  uint64_t before_ms = timespec_ms(before);
  uint64_t after_ms = timespec_ms(after);
  printf("Total time: %lld ms\n", after_ms - before_ms);

  enable_tracing();
  for (uint32_t i = 0; i < 1024000; i++) {
    set_remove(set, i);
  }
  flush_trace_hist();
  disable_tracing();

  TEST_ASSERT_EQUAL(0, debug_node_blackheight(set.nodes, set.colors, set.inited,
                                              set.root, true, true));
}
