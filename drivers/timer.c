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
 * Implementation of the board's timer functionality.
 * All 4 available timers are supported.
 *
 * More info about the board and the timer controller:
 * - Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 * - ARM Dual-Timer Module (SP804) Technical Reference Manual (DDI0271):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0271d/DDI0271.pdf
 *
 * @author Jernej Kovacic
 */

/*
 * TODO
 * Maybe publicly exposed functions should not be permitted to modify those settings that
 * trigger tick interrupts (IRQ4, timer(0, o))?
 */

#include <stdint.h>
#include <stddef.h>

#include "regutil.h"
#include "bsp.h"
#include "timer.h"


/* Number of counters per timer: */
#define NR_COUNTERS      ( 2U )


/*
 * Bit masks for the Control Register (TimerXControl).
 *
 * For description of each control register's bit, see page 3-2 of DDI0271:
 *
 *  31:8 reserved
 *   7: enable bit (1: enabled, 0: disabled)
 *   6: timer mode (0: free running, 1: periodic)
 *   5: interrupt enable bit (0: disabled, 1: enabled)
 *   4: reserved
 *   3:2 prescale (00: 1, other combinations are not supported)
 *   1: counter length (0: 16 bit, 1: 32 bit)
 *   0: one shot enable bit (0: wrapping, 1: one shot)
 */

#define CTL_ENABLE          ( 0x00000080U )
#define CTL_MODE            ( 0x00000040U )
#define CTL_INTR            ( 0x00000020U )
#define CTL_PRESCALE_1      ( 0x00000008U )
#define CTL_PRESCALE_2      ( 0x00000004U )
#define CTL_CTRLEN          ( 0x00000002U )
#define CTL_ONESHOT         ( 0x00000001U )


/*
 * 32-bit registers of each counter within a timer controller.
 * See page 3-2 of DDI0271:
 */
typedef struct
{
    uint32_t LOAD;                   /* Load Register, TimerXLoad */
    uint32_t VALUE;                  /* Current Value Register, TimerXValue, read only */
    uint32_t CONTROL;                /* Control Register, TimerXControl */
    uint32_t INTCLR;                 /* Interrupt Clear Register, TimerXIntClr, write only */
    uint32_t RIS;                    /* Raw Interrupt Status Register, TimerXRIS, read only */
    uint32_t MIS;                    /* Masked Interrupt Status Register, TimerXMIS, read only */
    uint32_t BGLOAD;                 /* Background Load Register, TimerXBGLoad */
    uint32_t Unused;                 /* Unused, should not be modified */
} SP804_COUNTER_REGS;


/*
 * 32-bit registers of individual timer controllers,
 * relative to the controllers' base address:
 * See page 3-2 of DDI0271:
 */
typedef volatile struct
{
    SP804_COUNTER_REGS CNTR[NR_COUNTERS];     /* Registers for each of timer's two counters */
    uint32_t Reserved1[944];                  /* Reserved for future expansion, should not be modified */
    uint32_t ITCR;                            /* Integration Test Control Register */
    uint32_t ITOP;                            /* Integration Test Output Set Register, write only */
    uint32_t Reserved2[54];                   /* Reserved for future expansion, should not be modified */
    uint32_t PERIPHID[4];                     /* Timer Peripheral ID, read only */
    uint32_t CELLID[4];                       /* PrimeCell ID, read only */
} ARM926EJS_TIMER_REGS;


/*
 * Pointers to all timer registers' base addresses:
 */
#define CAST_ADDR(ADDR)    (ARM926EJS_TIMER_REGS*) (ADDR),

static ARM926EJS_TIMER_REGS * const pReg[BSP_NR_TIMERS] =
                         {
                             BSP_TIMER_BASE_ADDRESSES(CAST_ADDR)
                         };

#undef CAST_ADDR


/**
 * Initializes all timer controller.
 */
void all_timer_init(void)
{
    uint8_t i, j;

    /* Init all counters of all available timers */
    for ( i = 0U; i < BSP_NR_TIMERS; ++i )
    {
        for ( j = 0U; j < NR_COUNTERS; ++j )
        {
            timer_init(i, j);
        }
    }
}

#ifdef DEBUG
#define CHECK_TIMER 1
#endif


/**
 * Initializes the specified timer's counter controller.
 * The following parameters are set:
 * - periodic mode (when the counter reaches 0, it is wrapped to the value of the Load Register)
 * - 32-bit counter length
 * - prescale = 1
 *
 * This function does not enable interrupt triggering and does not start the counter!
 *
 * Nothing is done if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 */
void timer_init(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }
#endif


    /*
     * DDI0271 does not recommend modifying reserved bits of the Control Register (see page 3-5).
     * For that reason, the register is set in two steps:
     * - the appropriate bit masks of 1-bits are bitwise or'ed to the CTL
     * - zero complements of the appropriate bit masks of 0-bits are bitwise and'ed to the CTL
     */


    /*
     * The following bits will be set to 1:
     * - timer mode (periodic)
     * - counter length (32-bit)
     */

    HWREG_SET_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, ( CTL_MODE | CTL_CTRLEN ) );

    /*
     * The following bits will be set to 0:
     * - enable bit (disabled, i.e. timer not running)
     * - interrupt bit (disabled)
     * - both prescale bits (00 = 1)
     * - oneshot bit (wrapping mode)
     */

    HWREG_CLEAR_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL,
                      ( CTL_ENABLE | CTL_INTR | CTL_PRESCALE_1 | CTL_PRESCALE_2 | CTL_ONESHOT ) );

    /* reserved bits remained unmodified */
}


/**
 * Starts the specified timer's counter.
 *
 * Nothing is done if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 */
void timer_start(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }
#endif

    /* Set bit 7 of the Control Register to 1, do not modify other bits */
    HWREG_SET_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_ENABLE );
}


#if 0
/**
 * Stops the specified timer's counter.
 *
 * Nothing is done if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 */
void timer_stop(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }
#endif

    /* Set bit 7 of the Control Register to 0, do not modify other bits */
    HWREG_CLEAR_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_ENABLE );
}


/**
 * Checks whether the specified timer's counter is enabled, i.e. running.
 *
 * If it is enabled, a nonzero value, typically 1, is returned,
 * otherwise a zero value is returned.
 *
 * If either 'timerNr' or 'counterNr' is invalid, a zero is returned
 * (as an invalid timer/counter cannot be enabled).
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 *
 * @return a zero value if the timer is disabled, a nonzero if it is enabled
 */
int8_t timer_isEnabled(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return 0;
    }
#endif

    /* just check the enable bit of the timer's Control Register */
    return ( 0!=HWREG_READ_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_ENABLE ) );
}
#endif


/**
 * Enables the timer's interrupt triggering (when the counter reaches 0).
 *
 * Nothing is done if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 */
void timer_enableInterrupt(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }
#endif

    /* Set bit 5 of the Control Register to 1, do not modify other bits */
    HWREG_SET_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_INTR );
}


#if 0
/**
 * Disables the timer's interrupt triggering (when the counter reaches 0).
 *
 * Nothing is done if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 */
void timer_disableInterrupt(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }
#endif

    /* Set bit 5 of the Control Register to 0, do not modify other bits */
    HWREG_CLEAR_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_INTR );
}
#endif


/**
 * Clears the interrupt output from the specified timer.
 *
 * Nothing is done if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 */
void timer_clearInterrupt(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }
#endif

    /*
     * Writing anything (e.g. 0xFFFFFFFF, i.e. all ones) into the
     * Interrupt Clear Register clears the timer's interrupt output.
     * See page 3-6 of DDI0271.
     */
    pReg[timerNr]->CNTR[counterNr].INTCLR = 0xFFFFFFFFU;
}


/**
 * Sets the value of the specified counter's Load Register.
 *
 * When the timer runs in periodic mode and its counter reaches 0,
 * the counter is reloaded to this value.
 *
 * For more details, see page 3-4 of DDI0271.
 *
 * Nothing is done if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 * @param value - value to be loaded int the Load Register
 */
void timer_setLoad(uint8_t timerNr, uint8_t counterNr, uint32_t value)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }
#endif

    pReg[timerNr]->CNTR[counterNr].LOAD = value;
}


#if 0
/**
 * Returns the value of the specified counter's Value Register,
 * i.e. the value of the counter at the moment of reading.
 *
 * Zero is returned if either 'timerNr' or 'counterNr' is invalid.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 *
 * @return value of the timer's counter at the moment of reading
 */
uint32_t timer_getValue(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return 0UL;
    }
#endif

    return pReg[timerNr]->CNTR[counterNr].VALUE;
}


/**
 * Address of the specified counter's Value Register. It might be suitable
 * for applications that poll this register frequently and wish to avoid
 * the overhead due to calling timer_getValue() each time.
 *
 * NULL is returned if either 'timerNr' or 'counterNr' is invalid.
 *
 * @note Contents at this address are read only and should not be modified.
 *
 * @param timerNr - timer number (between 0 and 1)
 * @param counterNr - counter number of the selected timer (between 0 and 1)
 *
 * @return read-only address of the timer's counter (i.e. the Value Register)
 */
volatile uint32_t* timer_getValueAddr(uint8_t timerNr, uint8_t counterNr)
{
#ifdef CHECK_TIMER
    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return NULL;
    }
#endif

    return (volatile uint32_t*) &(pReg[timerNr]->CNTR[counterNr].VALUE);
}


/**
 * @return number of counters per timer
 */
uint8_t timer_countersPerTimer(void)
{
    return NR_COUNTERS;
}
#endif
