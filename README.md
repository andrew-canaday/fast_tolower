fast\_tolower
=============

A neat trick for converting entire strings to lower case with minimal
branching and CPU usage.

Background: Fast Single-Character Conversions
---------------------------------------------

So there's this weird trick we can do to determine if a single 8-bit value
(c) lies within a range. That trick is as follows:
 * define LOW and HIGH to integer values defining the range (inclusive)
 * subtract the character value, c, from LOW
 * subtract the character value, c, from HIGH
 * XOR the difference

(Disclaimer: first made aware of this trick [here](http://codegolf.stackexchange.com/questions/36425/convert-to-uppercase-and-lowercase-without-branching-and-comparisons))

After performing this trick, we have the property that the top bit of the
resulting number will be set if `LOW <= c <= HIGH` (other bits may *also*
be set, but we *don't care*).

Next, we observe that the difference between upper and lowercase ascii
characters is (conveniently!) 32, i.e. a lowercase letter is equal to
its uppercase equivalent with the 6th bit set (e.g. `'a' == 'A' | 0x20`).

Using these two observations, we can flip an uppercase letter to lowercase
without branching, like so:

```C
/* Set c to some character value: */
char c = 'D'; // Any value works!

/* Here, the top bit is set if 0x40 ('A') <= c <= 0x5a ('Z')
 * We shift one place to the right so that we can leverage the bit
 * that's been conditionally set as a case-toggle and mask using 0x20
 * because we don't *care* about any of the other bits set in the XOR: */
uint8_t mask = (((0x40 - c) ^ (0x5a - c)) >> 1) & 0x20;

/* Now, we conditionally flip the 6th bit - changing to lowercase! */
c = c ^ mask;
```


The Algorithm
-------------

An interesting optimization presents itself, given the above bit-tricks.
Why operate on *one* character at a time, if we can extend the high, low,
and mask values to be multi-bit? This is precisely what we do below.

FAST_TOLOWER_STRIDE is set to the number of *bytes* in our systems largest
supported unsigned integer type. Using this information, we define MASK_S,
HIGH_S, and LOW_S to be the multi-character equivalent of the values used
in the discussion above, i.e. instead of checking one character at a time,
we cant test 2, 4, or 8 characters at a time, e.g.:

```C
/* Here, c2 is the two chars 'A' and 'B' next to each other in mem: */
uint16_t c2 = 'B' | (uint16_t)('A' << 8);

/* Perform the same trick at 16-bits: */
uint16_t mask = (((0x4040 - c2) ^ (0x5a5a - c2)) >> 1) & 0x2020;
c2 = c ^ mask;
```

Once we have this information, we can cast the input and output character
arrays to a suitably large integer type and iterate in strides the size
of the integer type, instead of byte-by-byte.

Many strings will end up not being neatly divisible by our maximum stride.
After we've run through as many characters as we can at integer stride,
we use a switch with intentional fallthrough to convert the remaining values
on a byte-by-byte basis without looping.


A Note on Alignment
-------------------

Simply converting the input string pointers to integer pointer types is the
obvious approach. However, on architectures with *strict alignment* this may
cause the program *to abort*. Many modern systems will allow you to violate
strict alignment rules, but as a result the machine generates extra
instructions to shift data across word boundaries before the computations
are performed.

To avoid this penalty, we determine *how many bytes away from a properly
aligned byte boundary* the input pointer is and handle this many bytes - one
at a time - ahead of time, until we reach the integer boundary.

Afterwards, we proceed with the integer-stride conversions and finish out
the computation by iterating over the remaining bytes (if any) byte-by-bye
once more.


Benchmark
---------

This repo includes a cobbled-together toy benchmarking program which can be
compiled and run, like so:

```sh
# Build with platform-native width (or default to 32-bit):
make benchmark

# A more fair comparison with proper architecture and compiler optimization:
CFLAGS="-march=native -O3" make check

# Build for a range of strides and test each:
FAST_CFLAGS="-O3 -Wall -Wno-format -march=native"
for s in 1 2 4 8 ; do
  CFLAGS="${FAST_CFLAGS} -DFAST_TOLOWER_STRIDE=$s" SUFFIX="$s" make check
done
```

LICENSE
-------
Licensed under the MIT license. See the LICENSE file for specifics.



