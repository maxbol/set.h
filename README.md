# set.h

Generic ordered set implementation using C.

* Single header implementation using macros
* Auto-balancing tree structure (Red & Black tree)
* Cache-efficient memory layout (entries kept in one single, expanding buffer)
* Bring your own hashing function 
* Small (~ 400 LOC)

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
