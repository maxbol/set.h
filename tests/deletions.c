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

void test_draw_tree(void) {
  set_uint32_t set;
  set_init(set, set_uint32_hash_fn);

  set_add(set, 2);
  set_add(set, 3);
  set_add(set, 4);
  set_add(set, 5);
  set_add(set, 6);
  set_add(set, 7);
  set_add(set, 18);
  set_add(set, 19);

  size_t canvas_width = 60;
  size_t canvas_height = 20;

  char out_buf[canvas_width * canvas_height * 64];
  int written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);

  set_remove(set, 7);

  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);

  set_add(set, 7);

  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);

  set_add(set, 8);
  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);
  set_add(set, 9);
  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);
  set_add(set, 10);
  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);
  set_add(set, 11);
  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);
  set_add(set, 12);
  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);
  set_add(set, 13);
  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);
  set_add(set, 14);
  TEST_ASSERT_NOT_EQUAL(
      set_node_blackheight(set.nodes, set.colors, set.inited, set.root, true),
      0);

  written =
      set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width,
                    canvas_height, out_buf, canvas_width * canvas_height * 64);

  printf("%.*s", written, out_buf);
}
