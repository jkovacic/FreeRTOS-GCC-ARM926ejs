/*
 * Since all ARM cores are functionally very similar, port.c from
 * the officially supported GCC/ARM7_LPC2000 port can be reused for
 * ARM926EJ-S too.
 *
 * prvSetupTimerInterrupt() was modified to handle timer and VIC properly
 * and minor modifications of the timer's ISR routine (vTickISR) were necessary.
 * Additionally all "annoying" tabs have been replaced by spaces.
 *
 * The original file is available under the following license:
 */

/*
 * FreeRTOS Kernel V10.4.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 */



/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM7 port.
 *
 * Components that can be compiled to either ARM or THUMB mode are
 * contained in this file.  The ISR routines, which can only be compiled
 * to ARM mode are contained in portISR.c.
 *----------------------------------------------------------*/



/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Includes for functions to peripherals' drivers: */
#include "bsp.h"
#include "interrupt.h"
#include "timer.h"
#include "tick_timer_settings.h"

/* Constants required to setup the task context. */
/* System mode, ARM mode, IRQ enabled, FIQ disabled */
#define portINITIAL_SPSR                ( ( StackType_t ) 0x5f )
#define portTHUMB_MODE_BIT              ( ( StackType_t ) 0x20 )
#define portINSTRUCTION_SIZE            ( ( StackType_t ) 4 )
#define portNO_CRITICAL_SECTION_NESTING ( ( StackType_t ) 0 )



/*-----------------------------------------------------------*/

/* Setup the timer to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/*
 * The scheduler can only be started from ARM mode, so
 * vPortISRStartFirstSTask() is defined in portISR.c.
 */
extern void vPortISRStartFirstTask( void );

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    StackType_t *pxOriginalTOS = pxTopOfStack;

    /* To ensure asserts in tasks.c don't fail, although in this case the assert
    is not really required. */
    pxTopOfStack--;

    /* Setup the initial stack of the task.  The stack is set exactly as
    expected by the portRESTORE_CONTEXT() macro. */

    /* First on the stack is the return address - which in this case is the
    start of the task.  The offset is added to make the return address appear
    as it would within an IRQ ISR. */
    *pxTopOfStack = ( StackType_t ) pxCode + portINSTRUCTION_SIZE;
    pxTopOfStack--;

    *pxTopOfStack = ( StackType_t ) 0xAAAAAAAAU;	/* R14 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pxOriginalTOS; /* Stack used when task starts goes in R13. */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x12121212U;	/* R12 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x11111111U;	/* R11 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x10101010U;	/* R10 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x09090909U;	/* R9 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x08080808U;	/* R8 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x07070707U;	/* R7 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x06060606U;	/* R6 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x05050505U;	/* R5 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x04040404U;	/* R4 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x03030303U;	/* R3 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x02020202U;	/* R2 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x01010101U;	/* R1 */
    pxTopOfStack--;

    /* When the task starts it will expect to find the function parameter in
     R0. */
    *pxTopOfStack = ( StackType_t ) pvParameters; /* R0 */
    pxTopOfStack--;

    /* The last thing onto the stack is the status register, which is set for
    system mode, with interrupts enabled. */
    *pxTopOfStack = ( StackType_t ) portINITIAL_SPSR;

    if( ( ( uint32_t ) pxCode & 0x01UL ) != 0x00UL )
    {
        /* We want the task to start in thumb mode. */
        *pxTopOfStack |= portTHUMB_MODE_BIT;
    }

    pxTopOfStack--;

    /* Some optimisation levels use the stack differently to others.  This
    means the interrupt flags cannot always be stored on the stack and will
    instead be stored in a variable, which is then saved as part of the
    tasks context. */
    *pxTopOfStack = portNO_CRITICAL_SECTION_NESTING;

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    /* Start the timer that generates the tick ISR.  Interrupts are disabled
    here already. */
    prvSetupTimerInterrupt();

    /* Start the first task. */
    vPortISRStartFirstTask();

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* It is unlikely that the ARM port will require this function as there
    is nothing to return to.  */
}
/*-----------------------------------------------------------*/


/*
 * Setup the timer 0 and the VIC
 */
static void prvSetupTimerInterrupt( void )
{
    /*
     * If timer settings are inappropriate (portTICK_TIMER>=BSP_NR_TIMERS), this
     * file will not compile. Thus an invalid timer's IRQ (something read from
     * a "random" location) will be prevented.
     */
#if portTICK_TIMER >= BSP_NR_TIMERS
#error Invalid timer selected!
#endif

    uint32_t ulCompareMatch;
#if 0
    const uint8_t irqs[BSP_NR_TIMERS] = BSP_TIMER_IRQS;
    const uint8_t irq = irqs[portTICK_TIMER]; /* This is IRQ4. */
#else
    const uint8_t irq = 4U;
#endif

    /* Calculate the match value required for our desired tick rate. */
    ulCompareMatch = ( 0U != configTICK_RATE_HZ ) ?
                       configCPU_CLOCK_HZ / configTICK_RATE_HZ :
                       (uint32_t) (-1);


    /* Counter's load should always be greater than 0 */
    if ( 0U == ulCompareMatch )
    {
        ulCompareMatch = 1U;
    }

    /* Configure the timer 0, counter 0 */
    timer_init(portTICK_TIMER, portTICK_TIMER_COUNTER);
    timer_setLoad(portTICK_TIMER, portTICK_TIMER_COUNTER, ulCompareMatch);
    timer_enableInterrupt(portTICK_TIMER, portTICK_TIMER_COUNTER);

    /* Configure the VIC to service IRQ4 (triggered by the timer) properly */
    (void) pic_registerIrq(irq, &vTickISR, PIC_MAX_PRIORITY);

    /* Enable servicing of IRQ4 */
    pic_enableInterrupt(irq);

    /*
     * Start the timer.
     * Note that IRQ mode will only be enabled when the first FreeRTOS task starts.
     */
    timer_start(portTICK_TIMER, portTICK_TIMER_COUNTER);

}
/*-----------------------------------------------------------*/
