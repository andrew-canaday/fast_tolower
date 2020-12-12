# Simple makefile for fast_tolower algorithm test.
CFLAGS ?= -O3 -Wall

.PHONY: all benchmark clean

all: benchmark

benchmark_tolower: main.c fast_tolower.h benchmark.h
	gcc $(CFLAGS) ./main.c -o benchmark_tolower

benchmark: benchmark_tolower
	./benchmark_tolower

clean:
	rm -vf *.o ./benchmark_tolower

# EOF

