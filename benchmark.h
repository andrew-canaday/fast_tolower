/*=============================================================================
 *
 * benchmark.h: Kludge benchmarking header for tests.
 *
 *===========================================================================*/

#ifndef FAST_TOLOWER_BENCHMARK_H
#define FAST_TOLOWER_BENCHMARK_H
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define USEC_PER_SEC 1000000

static struct timeval start_time;
static struct timeval stop_time;
static struct timeval pause_time;

/* Store the difference between to struct timevals: */
void timeval_subtract(
        struct timeval* result, struct timeval* start, struct timeval* end)
{
    result->tv_sec = end->tv_sec - start->tv_sec;
    if( end->tv_usec > start->tv_usec ) {
        result->tv_usec = end->tv_usec - start->tv_usec;
    }
    else {
        result->tv_sec -= 1;
        result->tv_usec = (USEC_PER_SEC - end->tv_usec) + start->tv_usec;
    };
    return;
}

void timeval_increment(struct timeval* out, struct timeval* diff)
{
    out->tv_sec += diff->tv_sec;
    long tv_usec = out->tv_usec + diff->tv_usec;
    if( tv_usec >= USEC_PER_SEC ) {
        ++out->tv_sec;
        out->tv_usec = USEC_PER_SEC - tv_usec;
    }
    else {
        out->tv_usec = tv_usec;
    };
    return;
};

/* Start timing an algorithm: */
static inline void benchmark_start()
{
    gettimeofday(&start_time, NULL);
    return;
}

/* Pause the timer, don't count any time elapsed between pause and unpause: */
static inline void benchmark_pause()
{
    gettimeofday(&pause_time, NULL);
    return;
}

/* Unpause the timer: */
static inline void benchmark_unpause()
{
    static struct timeval unpause_time;
    static struct timeval total_pause;
    gettimeofday(&unpause_time, NULL);
    timeval_subtract(&total_pause, &pause_time, &unpause_time);
#if 0
    printf("pause time: %lu.%06lus\n",
        (long)total_pause.tv_sec, (long)total_pause.tv_usec);
#endif
    timeval_increment(&start_time, &total_pause);
    return;
};

/* Stop the timer and spit out the total elapsed time: */
static inline void benchmark_stop()
{
    static struct timeval total_time;
    gettimeofday(&stop_time, NULL);
    timeval_subtract(&total_time, &start_time, &stop_time);
    printf("Total time: %lu.%06lus\n",
        (long)total_time.tv_sec, (long)total_time.tv_usec);
    return;
}

#endif /* FAST_TOLOWER_BENCHMARK_H */
/* EOF */

