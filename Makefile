UNITY_ROOT		:= ./Unity
CC		 				:= clang
DEPS 					:= set.h
CFLAGS 				:= -O0 -g -I. -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src -Wall -Werror

.PHONY: test clean all build_test

test: build_test
	./out/test/test_inserts
	./out/test/test_deletions

build_test: out/test/test_inserts out/test/test_deletions

clean: 
	rm -rf out/*

all: test
 
out/test/test_%: $(UNITY_ROOT)/src/unity.c tests/%.c test_runners/%.c set.h
	mkdir -p out/test
	$(CC) $(CFLAGS) -o $@ $(UNITY_ROOT)/src/unity.c tests/$*.c test_runners/$*.c $(SRC) 

test_runners/%.c: tests/%.c
	mkdir -p ./test_runners
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb $< $@
