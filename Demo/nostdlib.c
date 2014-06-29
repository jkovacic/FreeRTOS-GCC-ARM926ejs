/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
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

/* A convenience macro that defines the upper limit of 'size_t' */
#define SIZE_T_MAX     ( (size_t) (-1) )


/*
 * @param x - first value
 * @param y - second value
 *
 * @return smaller of both input values
 */
static inline size_t minval(size_t x, size_t y)
{
    return ( x<=y ? x : y );
}


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
void* memset(void* ptr, int value, size_t num )
{
    unsigned char* p = (unsigned char*) ptr;
    size_t n = num;

    /* sanity check */
    if ( NULL==ptr )
    {
        goto endf;
    }

    /*
     * If destination block exceeds the range of 'size_t',
     * decrease 'num' accordingly.
     */
    if ( num > (size_t) ((unsigned char*) SIZE_T_MAX - p) )
    {
        n = (unsigned char*) SIZE_T_MAX - p;
        /* TODO or maybe just goto endf???? */
    }

    /* Set 'value' to each byte of the block: */
    while (n--)
    {
        *(p++) = (unsigned char) value;
    }

endf:
    return ptr;
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
void* memcpy(void* destination, const void* source, size_t num )
{
    unsigned char* srcptr = (unsigned char*) source;
    unsigned char* destptr = (unsigned char*) destination;
    size_t n = num;

    /* sanity check */
    if ( NULL==source || NULL==destination )
    {
        return NULL;
    }

    /* Nothing to do if attempting to copy to itself: */
    if ( srcptr == destptr )
    {
        return destination;
    }

    /*
     * If any block exceeds the range of 'size_t',
     * decrease 'num' accordingly.
     */
    if ( num > (size_t) ((unsigned char*) SIZE_T_MAX-destptr) ||
         num > (size_t) ((unsigned char*) SIZE_T_MAX-srcptr) )
    {
        n = minval((unsigned char*) SIZE_T_MAX-destptr,
                   (unsigned char*) SIZE_T_MAX-srcptr);
        /* TODO or maybe just return destination? */
    }

    if ( destptr<srcptr || destptr>=(srcptr+n) )
    {
        /*
         * If blocks do not overlap or or backwards copy is requested,
         * it is safe to copy the source block from begin to end.
         */
        while (n--)
        {
            *destptr++ = *srcptr++;
        }
    }
    else
    {
        /*
         * If forward copy is requested and blocks overlap, forward copy
         * (from block's begin to end) would cause a corruption.
         * Hence backward copy (from end to begin) is performed.
         */
        srcptr += n - 1;
        destptr += n - 1;

        while (n--)
        {
            *destptr-- = *srcptr--;
        }
    }

    return destination;
}

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
char* strcpy (char* destination, const char* source)
{
    const char* srcptr = source;
    char* destptr = destination;

    /* sanity check */
    if ( NULL==destination || NULL==source )
    {
        return NULL;
    }

    while ( '\0' != *srcptr )
    {
        *destptr++ = *srcptr++;
    }

    /* Do not forget to append a '\0' at the end of destination! */
    *destptr = '\0';

    return destination;
}
