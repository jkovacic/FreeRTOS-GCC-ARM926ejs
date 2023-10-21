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
 * Definitions of base addresses and IRQs of ARM926EJ-S and Versatile
 * Application Baseboard peripherals.
 *
 * More details at:
 * Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 *
 * The header should be included into each source file that implements peripherals' drivers
 * or handles their interrupt requests.
 *
 * @author Jernej Kovacic
 */


/*
 * At the moment, this header file is maintained  manually.
 * Ideally, one day it will be generated automatically by scripts that
 * read data from BSP (board support package) databases.
 */


/*
 * Where multiple controllers of the same type are provided, their base addresses and IRQs
 * are defined as arrays. For IRQs this is pretty straightforward and arrays are simply defined
 * as values within curly brackets (e.g. "{ 3, 5, 8}"). That way it is simple to initialize
 * a statically declared array in source files where IRQs are necessary.
 * However, arrays of base addresses are a bit more tricky as addresses should be casted to
 * appropriate pointers types when assigned to pointers. This can be achieved using so called
 * "for-each macros". Within a macro definition, all addresses are enumerated as arguments to
 * another macro, e.g. CAST. Macros that are replaced by "CAST" are then defined in source files
 * when they are actually casted. For more details about this trick, see:
 * http://cakoose.com/wiki/c_preprocessor_abuse
 */


#ifndef _BSP_H_
#define _BSP_H_


#define MY_UINT8_MAX ((uint8_t) 255U)


/*
 * Base addresses and IRQs of all 3 UARTs
 * (see page 4-68 of the DUI0225D):
 */

#define BSP_NR_UARTS        ( 3U )

#define BSP_UART_BASE_ADDRESSES(CAST) \
    CAST( (0x101F1000) ) \
    CAST( (0x101F2000) ) \
    CAST( (0x101F3000) )

#define BSP_UART_IRQS       { ( 12 ), ( 13 ), ( 14 ) }




/*
 * Base address and IRQs of both timer controllers
 * (see pp. 4-21  and 4-67 of DUI0225D):
 */

#define BSP_NR_TIMERS       ( 2U )

#define BSP_TIMER_BASE_ADDRESSES(CAST) \
    CAST( (0x101E2000) ) \
    CAST( (0x101E3000) )

#define BSP_TIMER_IRQS      { ( 4 ), ( 5 ) }



#if 0
/*
 * Base address and IRQ of the built-in real time clock (RTC) controller
 * (see page 4-60 of the DUI0225D):
 */
#define BSP_RTC_BASE_ADDRESS        ( 0x101E8000 )

#define BSP_RTC_IRQ                 ( 10 )



/*
 * Base address and IRQ of the built-in watchdog controller
 * (see page 4-72 of the DUI0225D):
 */
#define BSP_WATCHDOG_BASE_ADDRESS   ( 0x101E1000 )

#define BSP_WATCHDOG_IRQ            ( 0 )



/*
 * IRQ, reserved for software generated interrupts.
 * See pp.4-46 to 4-48 of the DUI0225D.
 */

#define BSP_SOFTWARE_IRQ            ( 1 )
#endif

void hw_init(void);


#endif   /* _BSP_H_ */
