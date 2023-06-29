#pragma GCC diagnostic ignored "-Wredundant-decls"

/*
 * Copyright 2013, 2017, Jernej Kovacic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * @file
 * A collection of stdlib "clones", required by FreeRTOS.
 *
 * If the standard C library is going to be linked to the application,
 * do not link this file!
 *
 * @author Jernej Kovacic
 */

#include <stddef.h>
#include <string.h>


/**
 * Fill block of memory.
 *
 * Sets the first 'num' bytes of the block of memory pointed by 'ptr' to the
 * specified 'value' (interpreted as an unsigned char).
 *
 * @param ptr - pointer to the block of memory to fill
 * @param value - value to be set, passed as an int and converted to unsigned char
 * @param num - number of bytes to be set to the 'value'.
 *
 * @return 'ptr' is returned
 */
void * memset(void *m, int c, size_t n) __attribute__((used));

void * memset(void *m, int c, size_t n)
{
    char *s = (char *) m;

    while (n--)
      *s++ = (char) c;

    return m;
}


/**
 * Copy block of memory.
 *
 * Copies the values of 'num' bytes from the location pointed by 'source'
 * directly to the memory block pointed by 'destination'.
 *
 * The underlying type of the objects pointed by both the 'source' and
 * 'destination' pointers are irrelevant for this function; The result is
 * a binary copy of the data.
 *
 * The function does not check for any terminating null character in 'source' -
 * it always copies exactly 'num' bytes.
 *
 * If any block exceeds range of 'size_t', 'num' is decreased accordingly.
 *
 * The function copies the source block correctly even if both blocks overlap.
 *
 * @param destination - pointer to the destination array where the content is to be copied
 * @param source - pointer to the source of data to be copied
 * @param num - number of bytes to copy
 *
 * @return 'destination' is returned or NULL if any parameter equals NULL
 */
void * memcpy(void *dest, const void *src, size_t n) __attribute__((used));

void * memcpy(void *dest, const void *src, size_t n)
{
    char *destc = dest;
    const char *srcc = src;
    size_t i;

    for (i = 0; i < n; i++)
        destc[i] = srcc[i];

    return dest;
}


#if __OPTIMIZE_SIZE__
/**
 * Copy string.
 *
 * Copies the C string pointed by 'source' into the array pointed by
 * 'destination', including the terminating null character (and stopping
 * at that point).
 *
 * To avoid overflows, the size of the array pointed by destination shall be
 * long enough to contain the same C string as source (including the
 * terminating null character), and should not overlap in memory with source.
 *
 * @param destination - pointer to the destination array where the content is to be copied
 * @param source - C string to be copied
 *
 * @return 'destination' is returned or NULL if any parameter equals NULL
 */
char * strcpy(char *dst0, const char *src0)
{
    char *s = dst0;

    while ((*dst0++ = *src0++))
        ;

    return s;
}
#endif
