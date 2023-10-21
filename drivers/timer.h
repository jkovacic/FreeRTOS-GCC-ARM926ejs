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
 * the board's timer controller.
 *
 * @author Jernej Kovacic
 */


#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>


void all_timer_init(void);
void timer_init(uint8_t timerNr, uint8_t counterNr);
void timer_start(uint8_t timerNr, uint8_t counterNr);
#if 0
void timer_stop(uint8_t timerNr, uint8_t counterNr);
int8_t timer_isEnabled(uint8_t timerNr, uint8_t counterNr);
#endif
void timer_enableInterrupt(uint8_t timerNr, uint8_t counterNr);
#if 0
void timer_disableInterrupt(uint8_t timerNr, uint8_t counterNr);
#endif
void timer_clearInterrupt(uint8_t timerNr, uint8_t counterNr);
void timer_setLoad(uint8_t timerNr, uint8_t counterNr, uint32_t value);
#if 0
uint32_t timer_getValue(uint8_t timerNr, uint8_t counterNr);
const volatile uint32_t* timer_getValueAddr(uint8_t timerNr, uint8_t counterNr);
uint8_t timer_countersPerTimer(void);
#endif


#endif  /* _TIMER_H_*/
