/*
Copyright 2014, Jernej Kovacic

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
#define MASK_ONE           ( 0x00000001 )


/**
 * All bits of "reg", whose equivalent "mask" bits equal 1,
 * are set to 1. All other "reg" bits remain unmodified.
 */
#define HWREG_SET_BITS(reg, mask)        reg |= (mask);


/**
 * All bits of "reg", whose equivalent 'mask' bits equal 1,
 * are cleared to 0. All other "reg" bits remain unmodified.
 */
#define HWREG_CLEAR_BITS(reg, mask)      reg &= ~(mask);


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


/**
 * Returns status of all "reg" bits, whose equivalent "mask" bits
 * equal 1. Statuses of all other "reg" bits are returned as 0.
 */
#define HWREG_READ_BITS(reg, mask)       ( reg & (mask) )


/**
 * Returns a mask with a "bit"'th least significant bit set.
 */
#define HWREG_SINGLE_BIT_MASK(bit)       ( MASK_ONE << bit )


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
