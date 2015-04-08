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

/* The fast version is CHUNKY and live separately from the benchmark here: */
#include "fast_tolower.h"

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

/*========================================*
 *==- Ugly timing stuff for profiling: -==*
 *========================================*/

/* The timeing stuff was all stolen, ALMOST VERBATIM, from:
 * http://stackoverflow.com/questions/16764276/measuring-time-in-millisecond-precision
 */
static struct timeval tm1;
static struct timeval pausetime;

/* Start timing an algorithm: */
static inline void start()
{
    gettimeofday(&tm1, NULL);
}

/* Pause the timer, don't count any time elapsed between pause and unpause: */
static inline void pause()
{
    gettimeofday(&pausetime, NULL);
}

/* Unpause the timer: */
static inline void unpause()
{
    struct timeval tm2;
    gettimeofday(&tm2, NULL);
    tm1.tv_usec += (tm2.tv_usec - pausetime.tv_usec);
    tm1.tv_sec += (tm2.tv_sec - pausetime.tv_sec);
};

/* Stop the timer and spit out the total elapsed time: */
static inline void stop()
{
    struct timeval tm2;
    gettimeofday(&tm2, NULL);

    unsigned long long t = 1000 * (tm2.tv_sec - tm1.tv_sec) + \
                            (tm2.tv_usec - tm1.tv_usec) / 1000;
    fprintf(stderr, "%llu ms\n", t);
}

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
    int no_iter = 100000;
    char buffer[BUFF_SIZE];
    size_t len;

    if( argc > 1 ) {
        no_iter = atoi(argv[1]);
    };

    /* Get some random stuff: */
    srand(time(NULL));

    /* fast_tolower: */
    puts("Timing fast tolower...");
    start();
    for( i=0; i<no_iter; ++i )
    {
        pause();
        len = randomize(buffer, BUFF_SIZE);
        unpause();
        fast_tolower(buffer, buffer, len);
    };
    stop();
  
    /* slicker_tolower: */
    puts("Timing slicker tolower...");
    start();
    for( i=0; i<no_iter; ++i )
    {
        pause();
        len = randomize(buffer, BUFF_SIZE);
        unpause();
        slicker_tolower(buffer, buffer, len);
    };
    stop();

    /* naive_tolower: */
    puts("Timing naive tolower...");
    start();
    for( i=0; i<no_iter; ++i )
    {
        pause();
        len = randomize(buffer, BUFF_SIZE);
        unpause();
        naive_tolower(buffer, buffer, len);
    };
    stop();
    return 0;
};

/* EOF */

