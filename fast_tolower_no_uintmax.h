/*===========================================================================*
 *
 * fast_tolower.h: A neat trick for converting entire strings to lower case
 *                 with minimal branching and CPU usage.
 *
 * LICENSE:
 * --------
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Andrew T. Canaday
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *===========================================================================*/
#include <stdlib.h>
#include <stdint.h>

/*---------------------------------------------------------------------------*
 * Attempt a best guess for maximum stride, if unspecified:
 *---------------------------------------------------------------------------*/
#ifndef FAST_TOLOWER_STRIDE
    #if defined(SIZEOF_SIZE_T)
        /* Use autoconfg setting if sizeof(size_t) was found via:
         * AC_CHECK_SIZEOF([size_t])
         * AC_SUBST([SIZEOF_SIZE_T])
         */
        #define FAST_TOLOWER_STRIDE SIZEOF_SIZE_T
    #elif defined(__SIZEOF_SIZE_T__)
        /* Otherwise, use __SIZE_TYPE__: */
        #define FAST_TOLOWER_STRIDE __SIZEOF_SIZE_T__
    #else
        /* Otherwise, use the safest guess (32bit): */
        #warning "Unable to determine architecture! FAST_TOLOWER_STRIDE = 4 (default)."
        #define FAST_TOLOWER_STRIDE 4
    #endif /* SIZEOF_SIZE_T */
#endif /* FAST_TOLOWER_STRIDE */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
 * Set mask, high, and low values for max stride.
 * This is some gross-looking stuff we do so that we can leverage this hack on
 * 16-, 32-, and 64-bit systems:
 *---------------------------------------------------------------------------*/
#if FAST_TOLOWER_STRIDE == 8
    /* 64 bit systems */
    #define MASK_S 0x2020202020202020
    #define LOW_S 0x4040404040404040
    #define HIGH_S 0x5a5a5a5a5a5a5a5a
    typedef uint64_t stride_t;
#elif FAST_TOLOWER_STRIDE == 4
    /* 32 bit systems */
    #define MASK_S 0x20202020
    #define LOW_S 0x40404040
    #define HIGH_S 0x5a5a5a5a
    typedef uint32_t stride_t;
#elif FAST_TOLOWER_STRIDE == 2
    /* 16 bit systems */
    #define MASK_S 0x2020
    #define LOW_S 0x4040
    #define HIGH_S 0x5a5a
    typedef uint16_t stride_t;
#elif FAST_TOLOWER_STRIDE == 1
    /* 8 bit systems */
    #define MASK_S 0x20
    #define LOW_S 0x40
    #define HIGH_S 0x5a
    typedef uint8_t stride_t;
#endif /* FAST_TOLOWER_STRIDE */

/* Mask, high, low for single byte stride (arch-independent): */
#define MASK 0x20
#define LOW 0x40
#define HIGH 0x5a

/* Convenience macro for converting a single char to lowercase - pulled out
 * to make an already-ugly function slightly more readable: */
#define FAST_CHAR_TOLOWER(dst, src, tmp, mask) \
            tmp = *src++; \
            mask = (((LOW - tmp) ^ (HIGH - tmp)) >> 1) & MASK; \
            *dst++ = tmp ^ mask;


/*---------------------------------------------------------------------------*
 * Actual implementation:
 *---------------------------------------------------------------------------*/
static void fast_tolower( char* dst, const char* src, size_t len)
{
    /* Number of iterations we can perform at maximum stride: */
    size_t no_iter = len / FAST_TOLOWER_STRIDE;

    /* Number of single byte chunks to arrive at proper alignment: */
    size_t align = ((FAST_TOLOWER_STRIDE) - \
            ((stride_t)(src) % FAST_TOLOWER_STRIDE)) % FAST_TOLOWER_STRIDE;

    /* Number of single byte chunks after main no_iter is done: */
    size_t remain = (len % FAST_TOLOWER_STRIDE) - align;

    const stride_t* src_s;
    stride_t* dst_s;
    size_t i;
    stride_t mask_s;
    stride_t c_s;
    uint8_t mask;
    char c;

    /* Iterate byte-by-byte until we achieve proper alignment for stride_t: */
    switch( align ) {
    #if FAST_TOLOWER_STRIDE == 8
        case 7: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 6: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 5: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 4: FAST_CHAR_TOLOWER(dst, src, c, mask)
    #endif /* FAST_TOLOWER_STRIDE == 8 */
    #if FAST_TOLOWER_STRIDE >= 4
        case 3: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 2: FAST_CHAR_TOLOWER(dst, src, c, mask)
    #endif /* FAST_TOLOWER_STRIDE >= 4 */
        case 1: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 0:
        default:
            break;
    };

    /* Now iterate over the characters at the maximum stride possible: */
    src_s = (const stride_t*)src;
    dst_s = (stride_t*)dst;
    for( i=0; i<no_iter; ++i )
    {
        c_s = *src_s++;
        mask_s = (((LOW_S - c_s) ^ (HIGH_S - c_s)) >> 1) & MASK_S;
        *dst_s++ = c_s ^ mask_s;
    };

    /* Convert remaining characters individually: */
    src = ((const char*)src_s);
    dst = ((char*)dst_s);
    switch( remain ) {
    #if FAST_TOLOWER_STRIDE == 8
        case 7: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 6: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 5: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 4: FAST_CHAR_TOLOWER(dst, src, c, mask)
    #endif /* FAST_TOLOWER_STRIDE == 8 */
    #if FAST_TOLOWER_STRIDE >= 4
        case 3: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 2: FAST_CHAR_TOLOWER(dst, src, c, mask)
    #endif /* FAST_TOLOWER_STRIDE >= 4 */
        case 1: FAST_CHAR_TOLOWER(dst, src, c, mask)
        case 0:
        default:
            break;
    };
};

/* EOF */

