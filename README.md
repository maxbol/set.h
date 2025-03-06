# set.h

Generic ordered set implementation using C.

* Single header implementation using macros
* Auto-balancing tree structure (Red & Black tree)
* Cache-efficient memory layout (data types kept in singular, expanding buffers)
* Bring your own hashing function 
* Small (~ ~~400~~ ~~600~~ 1000 LOC)

## Why?

This is mostly for me learning C, data structures and algorithms. Probably not a very usable library for production purposes.

## Installation

Copy `set.h` into your project and include it into your C file.

## Usage
```c
#include "set.h"

// Define your set type 
typedef set_type(uint32_t) set_t;

uint64_t hash_fn(uint32_t value) { return value; }
bool equals_fn(uint32_t a, uint32_t b) { return a == b; }

int main(void) {
  // Initialize set, here with included default hashing function for 32-bit unsigned integers
  set_t set;
  set_init(set, hash_fn, equals_fn);

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

## Tracing

Include `trace.h` and `trace.c` in your build and build with `-DSET_TRACE_STEPS` in order to enable tracing for all operations in the set library. To flush all traces to stdout, use the included `flush_trace()` macro:

```c
for (uint32_t i = 0; i < 10; i++) {
  set_add(set, i);
  flush_trace();
}
```

Alternatively, you can flush the trace to a string buffer by using `sflush_trace()`.

```c
size_t trace_out_len = 1024 * 1024;
char trace_out[trace_out_len];
int bytes_written = sflush_trace(trace_out, trace_out_len);
```

## Interactive demo harness

The Makefile in this repo includes a live harness to interactively test the Red/Black tree. Build it by cloning the repo and running `make out/interactive_tester`.

Then start it by running `out/interactive_tester`.

The demo harness is using a set of uint32_t.

Available commands:
- `add [int]` - Add an integer entry to the set
- `remove [int]` - Remove integer entry [int] from the set
- `clear` - Empty set
- `print [line],[col]` - Print all characters (including control characters) from the ascii tree canvas at cursor position [line],[col].
- `back` - Rewind history by 1 step (up to a max of 5 steps from any given state).
- `next` - Forward history by 1 step.

## Unit tests

To run all unit tests in the repo, clone it and run `make test`.
