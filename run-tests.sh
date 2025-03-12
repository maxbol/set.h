#!/bin/bash

for test in $(find ./out/test -maxdepth 1 -type f); do
  echo "$test"
  $test || exit 1
done
