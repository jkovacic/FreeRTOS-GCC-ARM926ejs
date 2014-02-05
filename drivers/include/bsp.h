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
 *
 * Definitions of base addresses and IRQs of ARM926EJ-S and Versatile
 * Application Baseboard peripherals.
 *
 * More details at:
 * Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 *
 * The header should be included into each source file that implements peripherals' drivers
 * or handle their interrupt requests.
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



/* Base address of the Primary Interrupt Controller (see page 4-44 of the DUI0225D): */
#define BSP_PIC_BASE_ADDRESS        ( 0x10140000 )

/* Base address of the Secondary Interrupt Controller (see page 4-44 of the DUI0225D): */
#define BSP_SIC_BASE_ADDRESS        ( 0x10003000 )




/*
 * Base addresses and IRQs of all 3 UARTs
 * (see page 4-68 of the DUI0225D):
 */

#define BSP_NR_UARTS        ( 3 )

#define BSP_UART_BASE_ADDRESSES(CAST) \
    CAST( (0x101F1000) ) \
    CAST( (0x101F2000) ) \
    CAST( (0x101F3000) )

#define BSP_UART_IRQS       { ( 12 ), ( 13 ), ( 14 ) }




/*
 * Base address and IRQs of both timer controllers
 * (see pp. 4-21  and 4-67 of DUI0225D):
 */

#define BSP_NR_TIMERS       ( 2 )

#define BSP_TIMER_BASE_ADDRESSES(CAST) \
    CAST( (0x101E2000) ) \
    CAST( (0x101E3000) )

#define BSP_TIMER_IRQS      { ( 4 ), ( 5 ) }



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


#endif   /* _BSP_H_ */
