/*===========================================================================*
 *
 * benchmark_tolower: A cruddy benchmarking utility to test the fast_tolower
 *                    implementation.
 *
 *===========================================================================*/
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "benchmark.h"

/* The fast version is CHUNKY and live separately from the benchmark here: */
#include "fast_tolower.h"

#ifndef DEFAULT_NO_ITERATIONS
#define DEFAULT_NO_ITERATIONS 250000
#endif /* DEFAULT_NO_ITERATIONS */

#ifndef BUFF_SIZE
#define BUFF_SIZE 512
#endif /* BUFF_SIZE */

/* This is your typical naive implementation of full-string tolower, using the
 * tolower implementation from ctype.h: */
static void naive_tolower( char* dst, const char* src, size_t len)
{
    size_t i;
    for( i=0; i<len; ++i )
    {
        dst[i] = tolower(src[i]);
    };
    return;
};


/* This is a slightly slicker tolower implementation which uses a neato trick
 * to determine if a character lies on the interval [0x60, 0x5a] without
 * branching and conditionally sets the 0x20 bit if lowercase (see the comments
 * in fast_tolower.h, if you want a comprehensive breakdown: */
static void slicker_tolower( char* dst, const char* src, size_t len)
{
    size_t i;
    uint8_t m;
    char c;
    for( i=0; i<len; ++i )
    {
        c = src[i];
        m = (((0x40-c) ^ (0x5a-c)) >> 1) & 0x20;
        dst[i] = c ^ m;
    };
    return;
};

/* Used to generate a random character string: */
size_t randomize(char* buffer, size_t len)
{
    size_t no_chars = (size_t)rand() % (len-1);
    size_t i;
    for( i=0; i<no_chars; ++i )
    {
        buffer[i] = rand() % 0x100;
    };
    buffer[no_chars] = '\0';
    return no_chars;
};

/* Actually benchmark our functions: */
int main( int argc, char** argv )
{
    int i;
    int no_iter = DEFAULT_NO_ITERATIONS;
    char buffer[BUFF_SIZE];
    size_t len;
    struct timeval benchmark_time;

    if( argc > 1 ) {
        no_iter = atoi(argv[1]);
    };

    puts("\nRunning test with:");
    printf("\tMax string length: %i\n", BUFF_SIZE);
    printf("\tNumber of iterations: %i\n", no_iter);

    /* Get some random stuff: */
    srand(time(NULL));

    /* naive_tolower: */
    printf("%s", "Timing naive tolower...");
    benchmark_start();
    for( i=0; i<no_iter; ++i )
    {
        benchmark_pause();
        len = randomize(buffer, BUFF_SIZE);
        benchmark_unpause();
        naive_tolower(buffer, buffer, len);
    };
    benchmark_time = benchmark_stop();
    printf("%lu.%06lus\n",
        (long)benchmark_time.tv_sec, (long)benchmark_time.tv_usec);

    /* slicker_tolower: */
    printf("%s", "Timing slicker tolower...");
    benchmark_start();
    for( i=0; i<no_iter; ++i )
    {
        benchmark_pause();
        len = randomize(buffer, BUFF_SIZE);
        benchmark_unpause();
        slicker_tolower(buffer, buffer, len);
    };
    benchmark_time = benchmark_stop();
    printf("%lu.%06lus\n",
        (long)benchmark_time.tv_sec, (long)benchmark_time.tv_usec);

    /* fast_tolower: */
    printf("%s", "Timing fast tolower...");
    benchmark_start();
    for( i=0; i<no_iter; ++i )
    {
        benchmark_pause();
        len = randomize(buffer, BUFF_SIZE);
        benchmark_unpause();
        fast_tolower(buffer, buffer, len);
    };
    benchmark_time = benchmark_stop();
    printf("%lu.%06lus\n",
        (long)benchmark_time.tv_sec, (long)benchmark_time.tv_usec);

    return 0;
};

/* EOF */

