# Simple makefile for fast_tolower algorithm test.
CFLAGS ?= -O3 -Wall -Wno-format
SUFFIX ?= 

.PHONY: all check clean

all: check

benchmark: main.c fast_tolower.h benchmark.h
	gcc $(CFLAGS) ./main.c -o benchmark$(SUFFIX)

check: benchmark
	./benchmark$(SUFFIX)

clean:
	rm -vf *.o ./benchmark

# EOF

