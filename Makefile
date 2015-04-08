# Simple makefile for fast_tolower algorithm test.
benchmark_tolower: main.c fast_tolower.h
	gcc ./main.c -o benchmark_tolower

benchmark: benchmark_tolower
	./benchmark_tolower

clean:
	rm -vf *.o ./benchmark_tolower

# EOF

