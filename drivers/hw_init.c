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
 * HW initialization routines that should be called before startup section
 * calls main(). Functions from this file should not be publicly exposed
 * in headers.
 *
 * @author Jernej Kovacic
 */

#include <stdint.h>

#include "bsp.h"

#include "interrupt.h"
#include "timer.h"
#include "uart.h"

 /*
  * Performs initialization of all supported hardware.
  * All peripherals are stopped, their interrupt triggering is disabled, etc.
  */
void hw_init(void)
{
    /* Disable IRQ triggering (may be reenabled after ISRs are properly set) */
    /* irq_disableIrqMode(); This is already done in startup.s??? */

    /* Init the vectored interrupt controller */
    pic_init();

    /* Init all counters of all available timers */
    all_timer_init();

    /* Init all available UARTs */
    all_uart_init();
}
