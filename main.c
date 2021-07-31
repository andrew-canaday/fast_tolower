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

/* The fast version is CHUNKY and lives separately from the benchmark here: */
#if defined(FAST_TOLOWER_SIMD) && (FAST_TOLOWER_SIMD==1)
#include "simd_tolower.h"
#else
#include "fast_tolower.h"
#endif /* FAST_TOLOWER_SIMD */

#ifndef DEFAULT_NO_ITERATIONS
#define DEFAULT_NO_ITERATIONS 250000
#endif /* DEFAULT_NO_ITERATIONS */

#ifndef BUFF_SIZE
#define BUFF_SIZE 607
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


/* This is a slightly slicker tolower implementation which the trick described
 * in the README, but only does one byte at a time. */
static void slicker_tolower( char* dst, const char* src, size_t len)
{
    size_t i;
    uint8_t m;
    char c;
    for( i=0; i<len; ++i )
    {
        c = src[i];
        m = (((0x40-c) ^ (0x5a-c)) >> 2) & 0x20;
        dst[i] = c ^ m;
    };
    return;
};

/* Used to generate a random character string: */
size_t randomize_buffer(char* buffer, size_t len)
{
    size_t no_chars = len-1;
    size_t i;
    for( i=0; i<no_chars; ++i )
    {
        buffer[i] = rand() % ('Z' - 'A' + 1) + 'A';
    };
    buffer[no_chars] = '\0';
    return no_chars;
};

/* Used to generate a random character string: */
size_t randomize(char* buffer, size_t len)
{
    size_t no_chars = ((size_t)rand() % (len/2)) + (len/4);
    return randomize_buffer(buffer, no_chars);
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
    printf("    Max string length: %i\n", BUFF_SIZE);
    printf("    Number of iterations: %i\n", no_iter);
    printf("    Stride: %lu bytes\n", FAST_TOLOWER_STRIDE);

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
    printf("%s", "Timing slicker_tolower...");
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

    /* naive_tolower: */
    len = randomize_buffer(buffer, 32);
    puts("\nResults (naive_tolower):");
    printf("    before: %s\n", buffer);
    naive_tolower(buffer, buffer, len);
    printf("    after:  %s\n", buffer);

    /* fast_tolower: */
    len = randomize_buffer(buffer, 32);
    puts("\nResults (fast_tolower):");
    printf("    before: %s\n", buffer);
    fast_tolower(buffer, buffer, len);
    printf("    after:  %s\n", buffer);
    return 0;
};

/* EOF */

