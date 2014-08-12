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


/* Number of counters per timer: */
#define NR_COUNTERS      ( 2 )


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

#define CTL_ENABLE          ( 0x00000080 )
#define CTL_MODE            ( 0x00000040 )
#define CTL_INTR            ( 0x00000020 )
#define CTL_PRESCALE_1      ( 0x00000008 )
#define CTL_PRESCALE_2      ( 0x00000004 )
#define CTL_CTRLEN          ( 0x00000002 )
#define CTL_ONESHOT         ( 0x00000001 )


/*
 * 32-bit registers of each counter within a timer controller.
 * See page 3-2 of DDI0271:
 */
typedef struct _SP804_COUNTER_REGS
{
    uint32_t LOAD;                   /* Load Register, TimerXLoad */
    const uint32_t VALUE;            /* Current Value Register, TimerXValue, read only */
    uint32_t CONTROL;                /* Control Register, TimerXControl */
    uint32_t INTCLR;                 /* Interrupt Clear Register, TimerXIntClr, write only */
    uint32_t RIS;                    /* Raw Interrupt Status Register, TimerXRIS, read only */
    uint32_t MIS;                    /* Masked Interrupt Status Register, TimerXMIS, read only */
    uint32_t BGLOAD;                 /* Background Load Register, TimerXBGLoad */
    const uint32_t Unused;           /* Unused, should not be modified */
} SP804_COUNTER_REGS;


/*
 * 32-bit registers of individual timer controllers,
 * relative to the controllers' base address:
 * See page 3-2 of DDI0271:
 */
typedef struct _ARM926EJS_TIMER_REGS
{
    SP804_COUNTER_REGS CNTR[NR_COUNTERS];     /* Registers for each of timer's two counters */
    const uint32_t Reserved1[944];            /* Reserved for future expansion, should not be modified */
    uint32_t ITCR;                            /* Integration Test Control Register */
    uint32_t ITOP;                            /* Integration Test Output Set Register, write only */
    const uint32_t Reserved2[54];             /* Reserved for future expansion, should not be modified */
    const uint32_t PERIPHID[4];               /* Timer Peripheral ID, read only */
    const uint32_t CELLID[4];                 /* PrimeCell ID, read only */
} ARM926EJS_TIMER_REGS;


/*
 * Pointers to all timer registers' base addresses:
 */
#define CAST_ADDR(ADDR)    (ARM926EJS_TIMER_REGS*) (ADDR),

static volatile ARM926EJS_TIMER_REGS* const  pReg[BSP_NR_TIMERS]=
                         {
                             BSP_TIMER_BASE_ADDRESSES(CAST_ADDR)
                         };

#undef CAST_ADDR

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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }


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
     * The following bits are will be to 0:
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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }

    /* Set bit 7 of the Control Register to 1, do not modify other bits */
    HWREG_SET_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_ENABLE );
}


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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }

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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return 0;
    }

    /* just check the enable bit of the timer's Control Register */
    return ( 0!=HWREG_READ_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_ENABLE ) );
}


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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }

    /* Set bit 5 of the Control Register to 1, do not modify other bits */
    HWREG_SET_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_INTR );
}


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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }

    /* Set bit 5 of the Control Register to 0, do not modify other bits */
    HWREG_CLEAR_BITS( pReg[timerNr]->CNTR[counterNr].CONTROL, CTL_INTR );
}


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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }

    /*
     * Writing anything (e.g. 0xFFFFFFFF, i.e. all ones) into the
     * Interrupt Clear Register clears the timer's interrupt output.
     * See page 3-6 of DDI0271.
     */
    pReg[timerNr]->CNTR[counterNr].INTCLR = 0xFFFFFFFF;
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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return;
    }

    pReg[timerNr]->CNTR[counterNr].LOAD = value;
}


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

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return 0UL;
    }

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
const volatile uint32_t* timer_getValueAddr(uint8_t timerNr, uint8_t counterNr)
{

    /* sanity check: */
    if ( timerNr >= BSP_NR_TIMERS || counterNr >= NR_COUNTERS )
    {
        return NULL;
    }

    return (const volatile uint32_t*) &(pReg[timerNr]->CNTR[counterNr].VALUE);
}


/**
 * @return number of counters per timer
 */
uint8_t timer_countersPerTimer(void)
{
    return NR_COUNTERS;
}
