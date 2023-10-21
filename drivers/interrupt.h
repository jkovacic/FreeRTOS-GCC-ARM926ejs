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
 *
 * Declaration of public functions that handle
 * the board's primary interrupt controller (PIC).
 *
 * @author Jernej Kovacic
 */

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_


#include <stdint.h>

#define PIC_MAX_PRIORITY     ( 127U )

/**
 * Required prototype for vectored ISR servicing routines
 */
typedef void (*pVectoredIsrPrototype)(void);

#if 0
void irq_enableIrqMode(void);
#endif
void irq_disableIrqMode(void);
void _pic_IrqHandler(void);
void pic_init(void);
void pic_enableInterrupt(uint8_t irq);
#if 0
void pic_disableInterrupt(uint8_t irq);
void pic_disableAllInterrupts(void);
int8_t pic_isInterruptEnabled(uint8_t irq);
int8_t pic_getInterruptType(uint8_t irq);
void pic_setInterruptType(uint8_t irq, int8_t toIrq);
void pic_setDefaultVectorAddr(pVectoredIsrPrototype addr);
#endif
int8_t pic_registerIrq(uint8_t irq, pVectoredIsrPrototype addr, uint8_t priority);
#if 0
void pic_unregisterIrq(uint8_t irq);
void pic_unregisterAllIrqs(void);
int8_t pic_setSwInterruptNr(uint8_t irq);
int8_t pic_clearSwInterruptNr(uint8_t irq);
int8_t pic_setSoftwareInterrupt(void);
int8_t pic_clearSoftwareInterrupt(void);
#endif


#endif  /* _INTERRUPT_H_ */
