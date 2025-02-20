# set.h

Generic ordered set implementation using C.

* Single header implementation using macros
* Auto-balancing tree structure (Red & Black tree)
* Cache-efficient memory layout (data types kept in singular, expanding buffers)
* Bring your own hashing function 
* Small (~ 500 LOC)

## Why?

This is mostly for me learning C, data structures and algorithms. Probably not a very usable library for production purposes.

## Installation

Copy `set.h` into your project and include it into your C file.

## Usage
```c
#include "set.h"

// Define your set type 
typedef set_type(uint32_t) uint32_set_t;

int main(void) {
  // Initialize set, here with included default hashing function for 32-bit unsigned integers
  uint32_set_t set;
  set_init(set, set_uint32_hash_fn);

  // Add some integers to the set
  set_add(set, 2);
  set_add(set, 2); // Does not affect the state of the set
  set_add(set, 5);
  set_add(set, 7);

  // Get the size of the set
  size_t size = set_size(set); // 3

  // Check if member exists in set
  if (set_has(set, 5)) {
    // ... do something
  }

  // Iterate over members in set by getting the first set member and then using set_next
  size_t cursor = set_first(set);
  while (cursor != 0) {
    uint32_t entry = set_get_entry(set, cursor);
    cursor = set_next(set, cursor);
    // Do something with the entry...
  }

  // Remove set entry
  set_remove(set, 2);

  // Free set
  set_free(set);
}
```
## Debugging

A separate `setdebug.c` file (with corresponding header) is included in the source for debugging purposes.

### `set_node_blackheight()`
Use to get the black-height of any node in your set. Asserts that the black-height is uniform regardless of path travelled.

```c
set_uint32_t set;
set_init(set, set_uint32_hash_fn);

// Add a bunch of nodes
set_add(set, 1);
set_add(set, 10);
set_add(set, 6);

// Get blackheight of entire tree
size_t set_blackheight = set_node_blackheight(set.nodes, set.colors, set.inited, set.root, false);

// Get blackheight of a single node
size_t target_idx = set_get_node(set, set.root)->right;
size_t set_blackheight = set_node_blackheight(set.nodes, set.colors, set.inited, target_idx, false);
```

### `set_draw_tree()`
Draw an ASCII representation of the tree structure of the set.
```c
set_uint32_t set;
set_init(set, set uint32_hash_fn);

// Add a bunch of nodes
set_add(set, 7);
set_add(set, 11);
set_add(set, 105);
set_add(set, 2);

// Define canvas width and height in ASCII cursor positions
size_t canvas_width = 40;
size_t canvas_height = 10;

// Create a buffer to output the tree to, that is canvas_width * canvas_height * 64 chars wide.
size_t canvas_byte_width = canvas_width * canvas_height * 64;
char output_buf[canvas_byte_width];

// Write tree representation into output buffer. Returned value is number of bytes written.
int written = set_draw_tree(set.nodes, set.colors, set.inited, set.root, canvas_width, canvas_height, output_buf, canvas_byte_width);

// Print the tree to stdout
printf("%.*s", written, out_buf);
```
