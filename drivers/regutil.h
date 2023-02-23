/*
 * Copyright 2014, 2017, Jernej Kovacic
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
 *
 * Most driver routines require that only certain bits of a register
 * are read or modified, while all other bits remain unmodified.
 * To facilitate this, this header file contains a few convenience
 * macro definitions that perform most common operations, such as
 * modification of setting or clearing of individual bits, etc.
 *
 * This header file should be included to all driver source files
 * that perform bitwise operations on registers.
 *
 * @author Jernej Kovacic
 */

#ifndef _REGUTIL_H_
#define _REGUTIL_H_

/**
 * A definition with a LSB set, it can be shifted left
 * to represent any single bit.
 */
#define MASK_ONE           ( 0x00000001U )


/**
 * All bits of "reg", whose equivalent "mask" bits equal 1,
 * are set to 1. All other "reg" bits remain unmodified.
 */
#define HWREG_SET_BITS(reg, mask)        reg |= (mask)


/**
 * All bits of "reg", whose equivalent 'mask' bits equal 1,
 * are cleared to 0. All other "reg" bits remain unmodified.
 */
#define HWREG_CLEAR_BITS(reg, mask)      reg &= ~(mask)


#if 0
/**
 * First all "reg" bits, whose equivalent "mask" bits equal 1,
 * are cleared to 0.
 * Then all "reg" bits, whose equivalent "value" and "mask" bits
 * both equal 1, are set to 1.
 * All other "reg" bits remain unmodified.
 */
#define HWREG_SET_CLEAR_BITS(reg, value, mask) \
        reg &= ~( mask );   \
        reg |= ( (value) & (mask) );
#endif


/**
 * Returns status of all "reg" bits, whose equivalent "mask" bits
 * equal 1. Statuses of all other "reg" bits are returned as 0.
 */
#define HWREG_READ_BITS(reg, mask)       ( reg & (mask) )


/**
 * Returns a mask with a "bit"'th least significant bit set.
 */
#define HWREG_SINGLE_BIT_MASK(bit)       ( MASK_ONE << (bit) )


/**
 * The "bit"'th least significant bit of "reg" is set to 1.
 * All other "reg" bits remain unmodified.
 */
#define HWREG_SET_SINGLE_BIT(reg, bit)   HWREG_SET_BITS(reg, MASK_ONE<<(bit))


/**
 * The "bit"'th least significant bit of "reg" is cleared to 0.
 * All other "reg" bits remain unmodified.
 */
#define HWREG_CLEAR_SINGLE_BIT(reg, bit) HWREG_CLEAR_BITS(reg, MASK_ONE<<(bit))


/**
 * Returns status of the "REG"'s "bit"'th least significant bit. Statuses
 * of all other "reg"'s bits are returned as 0.
 */
#define HWREG_READ_SINGLE_BIT(reg, bit)  HWREG_READ_BITS(reg, MASK_ONE<<(bit))


#endif   /* _REGUTIL_H_ */
