# Simple makefile for fast_tolower algorithm test.
CC ?= gcc
CFLAGS ?= -O3 -Wall -Wno-format -march=native
SUFFIX ?= 

.PHONY: all check clean

all: benchmark

benchmark: main.c fast_tolower.h benchmark.h
	$(CC) $(CFLAGS) ./main.c -o benchmark$(SUFFIX)

check: benchmark
	./benchmark$(SUFFIX)

clean:
	rm -vf *.o ./benchmark

# EOF

