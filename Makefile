UNITY_ROOT		:= ./Unity
CC		 				:= clang
DEPS 					:= set.h
CFLAGS 				:= -O0 -g -I. -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src -Wall -fmacro-backtrace-limit=0

.PHONY: test clean all build_test
.PRECIOUS: test_runners/%.c

test: build_test
	./run-tests.sh

build_test: $(patsubst tests/%.c,out/test/test_%,$(wildcard tests/*.c)) 

clean: 
	rm -rf out/*

all: build_test out/interactive_tester

out/interactive_tester: set.h setdebug.h setdebug.c trace.c trace.h interactive_tester/main.c
	mkdir -p out
	$(CC) $(CFLAGS) -o $@ interactive_tester/main.c setdebug.c trace.c -DSET_TRACE_STEPS -Werror
 
out/test/test_%: $(UNITY_ROOT)/src/unity.c tests/%.c test_runners/%.c setdebug.c trace.c setdebug.h trace.h set.h
	mkdir -p out/test
	$(CC) $(CFLAGS) -o $@ $(UNITY_ROOT)/src/unity.c tests/$*.c test_runners/$*.c setdebug.c trace.c -DSET_TRACE_STEPS

test_runners/%.c: tests/%.c
	mkdir -p ./test_runners
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb $< $@
