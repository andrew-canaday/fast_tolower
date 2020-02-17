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
 *
 * Background: Fast Single-Character Conversions:
 * ----------------------------------------------
 *
 * So there's this weird trick we can do to determine if a single 8-bit value
 * (c) lies within a range. That trick is as follows:
 *  - define LOW and HIGH to integer values defining the range (inclusive)
 *  - subtract the character value, c, from LOW
 *  - subtract the character value, c, from HIGH
 *  - XOR the difference
 *
 *  (Disclaimer: first made aware of this trick here:
 *  http://codegolf.stackexchange.com/questions/36425/convert-to-uppercase-and-lowercase-without-branching-and-comparisons)
 *
 * After performing this trick, we have the property that the top bit of the
 * resulting number will be set if LOW <= c <= HIGH (other bits may *also* be
 * set, but we *don't care*).
 *
 * Next, we observe that the difference between upper and lowercase ascii
 * characters is (conveniently!) 32, i.e. a lowercase letter is equal to
 * its uppercase equivalent with the 6th bit set (e.g. 'a' = 'A' | 0x20).
 *
 * Using these two observations, we can flip an uppercase letter to lowercase
 * without branching, like so:
 *
 *      // Set c to some character value:
 *      char c = 'D'; // Any value works!
 *
 *      // Here, the top bit is set if 0x40 ('A') <= c <= 0x5a ('Z')
 *      // We shift one place to the right so that we can leverage the bit
 *      // that's been conditionally set as a case-toggle and mask using 0x20
 *      // because we don't *care* about any of the other bits set in the XOR:
 *      uint8_t mask = (((0x40 - c) ^ (0x5a - c)) >> 1) & 0x20;
 *
 *      // Now, we conditionally flip the 6th bit - changing to lowercase!
 *      c = c ^ mask;
 *
 *
 * The Algorithm:
 * --------------
 *
 * An interesting optimization presents itself, given the above bit-tricks.
 * Why operate on *one* character at a time, if we can extend the high, low,
 * and mask values to be multi-bit? This is precisely what we do below.
 *
 * FAST_TOLOWER_STRIDE is set to the number of *bytes* in our systems largest
 * supported unsigned integer type. Using this information, we define MASK_S,
 * HIGH_S, and LOW_S to be the multi-character equivalent of the values used
 * in the discussion above, i.e. instead of checking one character at a time,
 * we cant test 2, 4, or 8 characters at a time, e.g.:
 *
 *      // Here, c2 is the two chars 'A' and 'B' next to each other in mem:
 *      uint16_t c2 = 'B' | (uint16_t)('A' << 8);
 *
 *      // Perform the same trick at 16-bits:
 *      uint16_t mask = (((0x4040 - c2) ^ (0x5a5a - c2)) >> 1) & 0x2020;
 *      c2 = c ^ mask;
 *
 * Once we have this information, we can cast the input and output character
 * arrays to a suitably large integer type and iterate in strides the size
 * of the integer type, instead of byte-by-byte.
 *
 * Many strings will end up not being neatly divisible by our maximum stride.
 * After we've run through as many characters as we can at integer stride,
 * we use a switch with intentional fallthrough to convert the remaining values
 * on a byte-by-byte basis without looping.
 *
 *
 * A Note on Alignment:
 * --------------------
 *
 * Simply converting the input string pointers to integer pointer types is the
 * obvious approach. However, on architectures with *strict alignment* this may
 * cause the program *to abort*. Many modern systems will allow you to voilate
 * strict alignment rules, but as a result the machine generates extra
 * instructions to shift data across word boundaries before the computations
 * are performed.
 *
 * To avoid this penalty, we determine *how many bytes away from a properly
 * aligned byte boundary* the input pointer is and handle this many bytes - one
 * at a time - ahead of time, until we reach the integer boundary.
 *
 * Afterwards, we proceed with the integer-stride conversions and finish out
 * the computation by iterating over the remaining bytes (if any) byte-by-bye
 * once more.
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
    /* 16 bit systems */
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

