#include <stdint.h>

#include "set.h"
#include "setdebug.h"
#include "unity.h"

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

  size_t canvas_width = 40;
  size_t canvas_height = 10;
  size_t out_len = canvas_width * canvas_height * 64;
  char out[out_len];

  int written = set_draw_tree(set.nodes, set.colors, set.inited, set.root,
                              canvas_width, canvas_height, out, out_len);
  printf("%.*s", written, out);
}
