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
 * Implementation of the board's Primary Interrupt Controller (PIC) functionality.
 *
 * Secondary Interrupt Controller (SIC) is currently not supported.
 *
 * More info about the board and the PIC controller:
 * - Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 * - PrimeCell Vectored Interrupt Controller (PL190) Technical Reference Manual (DDI0181):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0181e/DDI0181.pdf
 *
 * Useful details about the CPU's IRQ mode are available at:
 * - ARM9EJ-S, Technical Reference Manual (DDI0222):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0222b/DDI0222.pdf
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
#include "uart.h"

/* For public definitions of types: */
#include "interrupt.h"



/*
 * 32-bit registers of the Primary Interrupt Controller,
 * relative to the controller's base address:
 * See page 3-3 of DDI0181.
 *
 * Although not explicitly mentioned by DDI0181, there are gaps
 * among certain groups of registers. The gaps are filled by
 * Unused* "registers" and are treated as "should not be modified".
 */
typedef volatile struct
{
    uint32_t VICIRQSTATUS;            /* IRQ Status Register, read only */
    uint32_t VICFIQSTATUS;            /* FIQ Status Register, read only */
    uint32_t VICRAWINTR;              /* Raw Interrupt Status Register, read only */
    uint32_t VICINTSELECT;            /* Interrupt Select Register */
    uint32_t VICINTENABLE;            /* Interrupt Enable Register */
    uint32_t VICINTENCLEAR;           /* Interrupt Enable Clear Register */
    uint32_t VICSOFTINT;              /* Software Interrupt Register */
    uint32_t VICSOFTINTCLEAR;         /* Software Interrupt Clear Register */
    uint32_t VICPROTECTION;           /* Protection Enable Register */
    uint32_t Unused1[3];              /* Unused, should not be modified*/
    uint32_t VICVECTADDR;             /* Vector Address Register */
    uint32_t VICDEFVECTADDR;          /* Default Vector Address Register */
    uint32_t Unused2[50];             /* Unused, should not be modified */
    uint32_t VICVECTADDRn[16];        /* Vector Address Registers */
    uint32_t Unused3[48];             /* Unused, should not be modified */
    uint32_t VICVECTCNTLn[16];        /* Vector Control Registers */
    uint32_t Unused4[872];            /* Unused, should not be modified */
    uint32_t VICPERIPHID[4];          /* Peripheral Identification Registers, read only */
    uint32_t VICCELLID[4];            /* PrimeCell Identification Registers, read only */
} ARM926EJS_PIC_REGS;


/*
 * Register map of the SIC is provided, but it is commented out
 * until the SIC is actually supported.
 */
#if 0
/*
 * 32-bit Registers of the Secondary Interrupt Controller,
 * relative to the controller's base address:
 * See page 4-49 of DUI0225D
 *
 * Note that the SIC is implemented inside a FPGA and is
 * not equal to the PIC. It triggers interrupt 31 on the PIC.
 *
 * Although not explicitly mentioned by DUI0225D, there is a gap
 * among registers. The gap is filled by Unused1 and is treated
 * as "should not be modified".
 *
 * Note that some registers share their addresses. See #defines below.
 */
typedef volatile struct
{
    uint32_t SIC_STATUS;              /* Status of interrupt (after mask), read only */
    uint32_t SIC_RAWSTAT;             /* Status of interrupt (before mask), read only */
    uint32_t SIC_ENABLE;              /* Interrupt mask; also SIC_ENSET */
    uint32_t SIC_ENCLR;               /* Clears bits in interrupt mask */
    uint32_t SIC_SOFTINTSET;          /* Set software interrupt */
    uint32_t SIC_SOFTINTCLR;          /* Clear software interrupt */
    uint32_t Unused1[2];              /* Unused, should not be modified */
    uint32_t SIC_PICENABLE;           /* Read status of pass-through mask; also SIC_PICENSET */
    uint32_t SIC_PICENCLR;            /* Clear interrupt pass through bits */
} ARM926EJS_SIC_REGS;

/* SIC_ENSET (Set bits in interrupt mask) shares its address with SIC_ENABLE. */
#define SIC_ENSET       SIC_ENABLE
/* SIC_PICENSET (Set interrupt pass through bits) shares its address with SIC_PICENABLE. */
#define SIC_PICENSET    SIC_PICENABLE

#endif


#define UL0                    ( 0x00000000U )
#define ULFF                   ( 0xFFFFFFFFU )
/* #define BM_IRQ_PART         ( 0x0000001FU ) */
#define BM_VECT_ENABLE_BIT     ( 0x00000020U )

#define NR_VECTORS             ( 16U )
#define NR_INTERRUPTS          ( 32U )


/* Base address of the Primary Interrupt Controller (see page 4-44 of the DUI0225D): */
#define BSP_PIC_BASE_ADDRESS        ( 0x10140000 )

#if 0
/* Base address of the Secondary Interrupt Controller (see page 4-44 of the DUI0225D): */
#define BSP_SIC_BASE_ADDRESS        ( 0x10003000 )
#endif

static ARM926EJS_PIC_REGS * const pPicReg = (ARM926EJS_PIC_REGS*) (BSP_PIC_BASE_ADDRESS);
/* static ARM926EJS_SIC_REGS * const pSicReg = (ARM926EJS_SIC_REGS*) (BSP_SIC_BASE_ADDRESS); */



/*
 * A table with IRQs serviced by each VICVECTADDRn.
 * If a table's field is negative, its corresponding VICVECTADDRn presumably
 * does not serve any IRQ. In this case, the corresponding VICVECTCNTLn is
 * supposed to be set to 0 and its VICVECTADDRn should be set to __irq_dummyISR.
 */
typedef struct
{
    pVectoredIsrPrototype isr;    /* address of the ISR */
    uint8_t irq;                  /* IRQ handled by this record */
    uint8_t priority;             /* priority of this IRQ */
} isrVectRecord;

static isrVectRecord __irqVect[NR_INTERRUPTS];


#ifdef DEBUG
#define CHECK_INTERRUPT
#endif


#if 0
/**
 * Enable CPU's IRQ mode that handles IRQ interrupt requests.
 */
void irq_enableIrqMode(void)
{
    /*
     * To enable IRQ mode, bit 7 of the Program Status Register (CSPR)
     * must be cleared to 0. See pp. 2-15 to 2-17 of the DDI0222 for more details.
     * The CSPR can only be accessed using assembler.
     */

    __asm volatile("MRS r0, cpsr");        /* Read in the CPSR register. */
    __asm volatile("BIC r0, r0, #0x80");   /* Clear bit 8, (0x80) -- Causes IRQs to be enabled. */
    __asm volatile("MSR cpsr_c, r0");      /* Write it back to the CPSR register */
}


/**
 * Disable CPU's IRQ and FIQ mode that handle IRQ interrupt requests.
 */
void irq_disableIrqMode(void)
{
    /*
     * To disable IRQ mode, bit 7 of the Program Status Register (CSPR)
     * must be set to 1. See pp. 2-15 to 2-17 of the DDI0222 for more details.
     * The CSPR can only be accessed using assembler.
     */

    __asm volatile("MRS r0, cpsr");       /* Read in the CPSR register. */
    __asm volatile("ORR r0, r0, #0xC0");  /* Disable IRQ and FIQ exceptions. */
    __asm volatile("MSR cpsr_c, r0");     /* Write it back to the CPSR register. */
}
#endif


/*
 * A dummy ISR routine for servicing vectored IRQs.
 *
 * It is supposed to be set as a default address of all vectored IRQ requests. If an "unconfigured"
 * IRQ is triggered, it is still better to be serviced by this dummy function instead of
 * being directed to an arbitrary address with possibly dangerous effects.
 */
static void __irq_dummyISR(void)
{
    /*
     * An "empty" function.
     * As this is a test application, it emits a warning to the UART0.
     */
     uart_print(0, "<WARNING, A DUMMY ISR ROUTINE!!!>\r\n");
}


/*
 * Default handler of vectored IRQs. Typically the address of this function should be
 * set as a default value to pPicReg->VICDEFVECTADDR. It handles IRQs whose ISRs are not
 * entered into vectored registers. It is very similar to non vectored handling of IRQs.
 */
static void __defaultVectorIsr(void)
{
    uint32_t vicintenable = pPicReg->VICINTENABLE;
    uint8_t cntr;

    /*
     * The current implementation assumes that the first 16 entries are properly serviced
     * and also enabled in their respective VICVECTCNTLn registers.
     */
    for ( cntr = NR_VECTORS; cntr < NR_INTERRUPTS; ++cntr )
    {
        if ( __irqVect[cntr].irq < NR_INTERRUPTS &&
             0U != HWREG_READ_SINGLE_BIT(vicintenable, __irqVect[cntr].irq ) )
        {
            ( *__irqVect[cntr].isr )();
            return;
        }
    }

    /*
     * The current implementation executes one ISR per call of this function.
     * If no appropriate ISR can be found, execute a dummy ISR.
     */
    __irq_dummyISR();
}


/*
 * IRQ handler routine, called directly from the IRQ vector, implemented in exception.c
 * Prototype of this function is not public and should not be exposed in a .h file. Instead,
 * its prototype must be declared as 'extern' where required (typically in exception.c only).
 *
 * NOTE:
 * There is no check that provided addresses are correct! It is up to developers
 * that valid ISR addresses are assigned before the IRQ mode is enabled!
 *
 * It supports two modes of IRQ handling, vectored and nonvectored mode. They are implemented
 * for testing purposes only, in a real world application, only one mode should be selected
 * and implemented.
 */
void _pic_IrqHandler(void)
{
    /*
     * Vectored implementation, a.k.a. "Vectored interrupt flow sequence", described
     * on page 2-9 of DDI0181.
     */

    /*
     * Reads the Vector Address Register with the ISR address of the currently active interrupt.
     * Reading this register also indicates to the priority hardware that the interrupt
     * is being serviced.
     */
    pVectoredIsrPrototype isrAddr = (pVectoredIsrPrototype) pPicReg->VICVECTADDR;

    /* Execute the routine at the vector address */
    (*isrAddr)();

    /*
     * Writes an arbitrary value to the Vector Address Register. This indicates to the
     * priority hardware that the interrupt has been serviced.
     */
    pPicReg->VICVECTADDR = ULFF;
}


/**
 * Initializes the primary interrupt controller to default settings.
 *
 * All interrupt request lines are set to generate IRQ interrupts and all
 * interrupt request lines are disabled by default. Additionally, all vector
 * and other registers are cleared.
 */
void pic_init(void)
{
    uint8_t i;

    /* All interrupt request lines generate IRQ interrupts: */
    pPicReg->VICINTSELECT = UL0;

    /* Disable all interrupt request lines: */
    pPicReg->VICINTENCLEAR = ULFF;

    /* Clear all software generated interrupts: */
    pPicReg->VICSOFTINTCLEAR = ULFF;

    /* Reset the default vector address: */
    pPicReg->VICDEFVECTADDR = (uint32_t) &__defaultVectorIsr;

    /* clear all vectored ISR addresses: */
    for ( i = 0U; i < NR_INTERRUPTS; ++i )
    {
        /* clear its entry in the table */
        __irqVect[i].isr = &__irq_dummyISR;    /* dummy ISR routine */
        __irqVect[i].irq = MY_UINT8_MAX;       /* no IRQ assigned */
        __irqVect[i].priority = MY_UINT8_MAX;  /* lowest priority */

        if ( i < NR_VECTORS )
        {
            /* clear its control register */
            pPicReg->VICVECTCNTLn[i] = UL0;
            /* and clear its ISR address to a dummy function */
            pPicReg->VICVECTADDRn[i] = (uint32_t) &__irq_dummyISR;
        }
    }
}


/**
 * Enable the the interrupt request line on the PIC for the specified interrupt number.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_enableInterrupt(uint8_t irq)
{
    /* TODO check for valid (unreserved) interrupt numbers? Applies also for other functions */

#ifdef CHECK_INTERRUPT
    if ( irq >= NR_INTERRUPTS )
    {
        return;
    }
#endif

    /* See description of VICINTENABLE, page 3-7 of DDI0181: */
    /* Only the bit for the requested interrupt source is modified. */
    HWREG_SET_SINGLE_BIT(pPicReg->VICINTENABLE, irq);
}


#if 0
/**
 * Disable the the interrupt request line on the PIC for the specified interrupt number.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_disableInterrupt(uint8_t irq)
{
#ifdef CHECK_INTERRUPT
    if ( irq >= NR_INTERRUPTS )
    {
        return;
    }
#endif

    /*
     * VICINTENCLEAR is a write only register and any attempt of reading it
     * will result in a crash. For that reason, operators as |=, &=, etc.
     * are not permitted and only direct setting of it (using operator =)
     * is possible. This is not a problem anyway as only 1-bits disable their
     * corresponding IRQs while 0-bits do not affect their corresponding
     * interrupt lines.
     *
     * For more details, see description of VICINTENCLEAR on page 3-7 of DDI0181.
     */
    pPicReg->VICINTENCLEAR = HWREG_SINGLE_BIT_MASK(irq);
}


/**
 * Disable all interrupt request lines of the PIC.
 */
void pic_disableAllInterrupts(void)
{
    /*
     * See description of VICINTENCLEAR, page 3-7 of DDI0181.
     * All 32 bits of this register are set to 1.
     */
    pPicReg->VICINTENCLEAR = ULFF;
}


/**
 * Checks whether the interrupt request line for the requested interrupt is enabled.
 *
 * 0 is returned if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return 0 if disabled, a nonzero value (typically 1) if the interrupt request line is enabled
 */
int8_t pic_isInterruptEnabled(uint8_t irq)
{
#ifdef CHECK_INTERRUPT
    if ( irq >= NR_INTERRUPTS )
    {
        return 0;
    }
#endif

    /* See description of VICINTENCLEAR, page 3-7 of DDI0181: */
    return (0U != HWREG_READ_SINGLE_BIT(pPicReg->VICINTENABLE, irq));
}


/**
 * What type (IRQ or FIQ) is the requested interrupt of?
 *
 * 0 is returned if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return 0 if irq's type is FIQ, a nonzero value (typically 1) if the irq's type is IRQ
 */
int8_t pic_getInterruptType(uint8_t irq)
{
#ifdef CHECK_INTERRUPT
    if ( irq >= NR_INTERRUPTS )
    {
        return 0;
    }
#endif

    /*
     * See description of VICINTSELECT, page 3-7 of DDI0181.
     *
     * If the corresponding bit is set to 1, the interrupt's type is FIQ,
     * otherwise it is IRQ.
     */

    return 0U == HWREG_READ_SINGLE_BIT(pPicReg->VICINTSELECT, irq);
}


/**
 * Set the requested interrupt to the desired type (IRQ or FIQ).
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 * @param toIrq - if 0, set the interrupt's type to FIQ, otherwise set it to IRQ
 */
void pic_setInterruptType(uint8_t irq, int8_t toIrq)
{
#ifdef CHECK_INTERRUPT
    if ( irq >= NR_INTERRUPTS )
    {
        return;
    }
#endif

    /*
     * Only the corresponding bit must be modified, all other bits must remain unmodified.
     * For that reason, appropriate bitwise operators are applied.
     *
     * The interrupt's type is set via VICINTSELECT. See description
     * of the register at page 3-7 of DDI0181.
     */
    if ( 0 != toIrq )
    {
        /* Set the corresponding bit to 0 by bitwise and'ing bitmask's zero complement */
        HWREG_CLEAR_SINGLE_BIT( pPicReg->VICINTSELECT, irq );
    }
    else
    {
        /* Set the corresponding bit to 1 by bitwise or'ing the bitmask */
        HWREG_SET_SINGLE_BIT( pPicReg->VICINTSELECT, irq );
    }
}


/**
 * Assigns the default vector address (VICDEFVECTADDR).
 *
 * Nothing is done, if 'addr' is invalid, i.e. NULL
 *
 * @param addr - address of the default ISR
 */
void pic_setDefaultVectorAddr(pVectoredIsrPrototype addr)
{
#ifdef CHECK_INTERRUPT
    if ( NULL == addr )
    {
        return;
    }
#endif

    pPicReg->VICDEFVECTADDR = (uint32_t) addr;
}
#endif


/**
 * Registers a vector interrupt ISR for the requested interrupt request line.
 * The vectored interrupt is enabled by default.
 *
 * Nothing is done and -1 is returned if either 'irq' is invalid (must be less than 32)
 * or ISR's address is NULL.
 *
 * Entries are internally sorted in descending order by priority.
 * Entries with the same priority are additionally sorted by the time of registration
 * (entries registered earlier are ranked higher).
 * If 'irq' has already been registered, its internal entry will be overridden with
 * new values and resorted by priority.
 * The first 16 entries, sorted by priority, are automatically entered into appropriate vector
 * registers of the primary interrupt controller.
 *
 * @note IRQ handling should be completely disabled prior to calling this function!
 *
 * @param irq - interrupt number (must be smaller than 32)
 * @param addr - address of the ISR that services the interrupt 'irq'
 * @param priority - priority of handling this IRQ (higher value means higher priority), the actual priority
 *                   will be silently truncated to 127 if this value is exceeded.
 *
 * @return position of the IRQ handling entry within an internal table, a negative value if registration was unsuccessful
 */
int8_t pic_registerIrq(
                        uint8_t irq,
                        pVectoredIsrPrototype addr,
                        uint8_t priority )
{
    const uint8_t prior = priority & PIC_MAX_PRIORITY;
    uint8_t irqPos = MY_UINT8_MAX;
    uint8_t prPos = MY_UINT8_MAX;
    uint8_t i;

#ifdef CHECK_INTERRUPT
    /* sanity check: */
    if (irq>=NR_INTERRUPTS || NULL==addr )
    {
        return -1;
    }
#endif

    /*
     * The priority table is traversed and two values are obtained:
     * - irqPos: index of the existing 'irq' or the first "empty" line
     * - prPos: index of the first entry whose priority is not larger or equal than 'prior'
     * The entry will be inserted into prPos, prior to that, all entries between 'irqPos and 'prPos'
     * will be moved one line up or down.
     */

    for ( i=0U; i<NR_INTERRUPTS; ++i )
    {
        if ( irqPos == MY_UINT8_MAX && (__irqVect[i].irq == MY_UINT8_MAX || __irqVect[i].irq==irq) )
        {
            irqPos = i;
        }

        if ( prPos == MY_UINT8_MAX && (__irqVect[i].priority == MY_UINT8_MAX || __irqVect[i].priority < prior) )
        {
            prPos = i;
        }
    }

    /* just in case, should never occur */
    if ( irqPos == MY_UINT8_MAX || prPos == MY_UINT8_MAX )
    {
        return -1;
    }

    /* if prPos is less than irqPos, move all intermediate entries one line up */
    if ( irqPos > prPos )
    {
        for ( i=irqPos; i>prPos; --i )
        {
            __irqVect[i] = __irqVect[i-1];

            /* for i < 16 also update PIC's vector address and control registers */
            if ( i < NR_VECTORS )
            {
                if ( __irqVect[i].irq != MY_UINT8_MAX)
                {
                    pPicReg->VICVECTCNTLn[i] = __irqVect[i].irq | BM_VECT_ENABLE_BIT;
                    pPicReg->VICVECTADDRn[i] = (uint32_t) __irqVect[i].isr;
                }
                else
                {
                    /* if i^th line is "empty", clear the appropriate vector registers */
                    pPicReg->VICVECTCNTLn[i] = UL0;
                    pPicReg->VICVECTADDRn[i] = (uint32_t) &__irq_dummyISR;
                }
            }  /* if i < NR_VECTORS */
        }  /* for i*/
    }  /* if irqPos > prPos */

    /* if prPos is greater than irqPos, move all intermediate entries one line down */
    if ( prPos > irqPos )
    {
        /* however this does not include the entry at prPos, whose priority is less than prior!!! */
        --prPos;

        for ( i = irqPos; i < prPos; ++i )
        {
            __irqVect[i] = __irqVect[i+1];

            /* for i<16 also update PIC's vector address and control registers */
            if ( i<NR_VECTORS )
            {
                if ( __irqVect[i].irq != MY_UINT8_MAX)
                {
                    pPicReg->VICVECTCNTLn[i] = __irqVect[i].irq | BM_VECT_ENABLE_BIT;
                    pPicReg->VICVECTADDRn[i] = (uint32_t) __irqVect[i].isr;
                }
                else
                {
                    /* if i^th line is "empty", clear the appropriate vector registers */
                    pPicReg->VICVECTCNTLn[i] = UL0;
                    pPicReg->VICVECTADDRn[i] = (uint32_t) &__irq_dummyISR;
                }
            }  /* if i < NR_VECTORS */
        }  /* for i */
    }  /* if prPos > irqPos */

    /* finally fill the entry at 'prPos' with the input values */
    __irqVect[prPos].isr = addr;
    __irqVect[prPos].irq = irq;
    __irqVect[prPos].priority = prior;

    /* if prPos<16 also update the appropriate vector registers */
    if ( prPos < NR_VECTORS )
    {
        pPicReg->VICVECTCNTLn[prPos] = irq | BM_VECT_ENABLE_BIT;
        pPicReg->VICVECTADDRn[prPos] = (uint32_t) addr;
    }

    return (int8_t) prPos;
}


#if 0
/**
 * Unregisters a vector interrupt ISR for the requested interrupt request line.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32 or
 * no vector for the 'irq' exists.
 *
 * @note IRQ handling should be completely disabled prior to calling this function!
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_unregisterIrq(uint8_t irq)
{
    uint8_t pos;

#ifdef CHECK_INTERRUPT
    if ( irq >= NR_INTERRUPTS )
    {
        return;
    }
#endif

    /* Find the 'irq' in the priority table: */
    for ( pos=0U; pos<NR_INTERRUPTS; ++pos )
    {
        if ( __irqVect[pos].irq == irq )
        {
            break; /* out of for pos */
        }
    }

    /* Nothing to do if IRQ has not been found: */
    if ( pos>=NR_INTERRUPTS )
    {
        return;
    }

    /*
     * Shift all entries past 'pos' (including invalid ones) one line up.
     * This will override the entry at 'pos'.
     */
    for ( ; pos<NR_INTERRUPTS-1; ++pos )
    {
        __irqVect[pos] = __irqVect[pos+1];

        if ( pos<NR_VECTORS )
        {
            /* for pos<16 also update PIC's vector address and control registers */
            if ( __irqVect[pos].irq != MY_UINT8_MAX )
            {
                pPicReg->VICVECTCNTLn[pos] = __irqVect[pos].irq | BM_VECT_ENABLE_BIT;
                pPicReg->VICVECTADDRn[pos] = (uint32_t) __irqVect[pos].isr;
            }
            else
            {
                /* if pos^th line is "empty", clear the appropriate vector registers */
                pPicReg->VICVECTCNTLn[pos] = UL0;
                pPicReg->VICVECTADDRn[pos] = (uint32_t) &__irq_dummyISR;
            }
        }
    }

    /* And "clear" the last entry to "default" values (see also pic_init()): */
    __irqVect[NR_INTERRUPTS-1].isr = &__irq_dummyISR;   /* dummy ISR routine */
    __irqVect[NR_INTERRUPTS-1].irq = MY_UINT8_MAX;      /* no IRQ assigned */
    __irqVect[NR_INTERRUPTS-1].priority = MY_UINT8_MAX; /* lowest priority */
}



/**
 * Unregisters all vector interrupts.
 */
void pic_unregisterAllIrqs(void)
{
    uint8_t i;

    /* Clear all entries in the priority table */
    for ( i=0U; i<NR_VECTORS; ++i )
    {
        __irqVect[i].isr = &__irq_dummyISR;
        __irqVect[i].irq = MY_UINT8_MAX;
        __irqVect[i].priority = MY_UINT8_MAX;

        /* Clear all vector's VICVECTCNTLn and VICVECTADDRn registers: */
        if ( i<NR_VECTORS )
        {
            pPicReg->VICVECTCNTLn[i] = UL0;
            pPicReg->VICVECTADDRn[i] = (uint32_t) &__irq_dummyISR;
        }
    }
}


/**
 * Triggers a software generated interrupt. The chosen interrupt request line
 * must be enabled (masked) in order for the interrupt to be actually triggered.
 *
 * Nothing is done if 'irq' is invalid (equal or greater than 32).
 *
 * @note It is strongly recommended that only IRQs of disabled peripherals
 * (or at least peripherals with disabled interrupt triggering) are passed
 * to this function. It is strongly recommended to use IRQ 1, reserved for
 * software generated interrupts.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return 'irq' if interrupt successfully set, a negative value (typically -1) otherwise
 */
int8_t pic_setSwInterruptNr(uint8_t irq)
{
#ifdef CHECK_INTERRUPT
    if (irq>=NR_INTERRUPTS)
    {
        return -1;
    }
#endif

    /*
     * Interrupts can be software triggered via VICSOFTINT.
     * See description of the register on page 3-8 of DDI0181.
     */

    HWREG_SET_SINGLE_BIT( pPicReg->VICSOFTINT, irq );

    return irq;
}


/**
 * Clears an active interrupt via software.
 *
 * Nothing is done if 'irq' is invalid (equal or greater than 32) or the
 * requested interrupt is not active.
 *
 * @note It is only recommended to use this function if the interrupt has been
 * set via software using pic_setSwInterruptNr.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return 'irq' if interrupt successfully cleared, a negative value (typically -1) otherwise
 */
int8_t pic_clearSwInterruptNr(uint8_t irq)
{
    uint32_t bitmask;
    uint8_t retVal = MY_UINT8_MAX;

#ifdef CHECK_INTERRUPT
    if (irq>=NR_INTERRUPTS)
    {
        return -1;
    }
#endif

    /*
     * Interrupts can be software cleared via VICSOFTINTCLEAR.
     * See description of the register on page 3-8 of DDI0181.
     */

     bitmask = HWREG_SINGLE_BIT_MASK(irq);

     /*
      * Before the interrupt is cleared it is checked whether it is active.
      * TODO: should VICIRQSTATUS and VICFIQSTATUS be check instead of VICRAWINTR?
      */
     if ( 0U != HWREG_READ_BITS( pPicReg->VICRAWINTR, bitmask ) )
     {
         /* The interrupt is active, clear it
          * * The register is write only and should not be read. Only 1-bits clear
          * their corresponding IRQs, 0-bits have no effect on "their" IRQs.
          */

         pPicReg->VICSOFTINTCLEAR = bitmask;

         retVal = irq;
     }

     return retVal;
}


/**
 * Triggers the software generated interrupt (IRQ1).
 *
 * @return 'irq' if interrupt successfully set, a negative value (typically -1) otherwise
 */
int8_t pic_setSoftwareInterrupt(void)
{
    return pic_setSwInterruptNr(BSP_SOFTWARE_IRQ);
}


/**
 * Clears the software generated interrupt (IRQ1).
 *
 * @return 'irq' if interrupt successfully set, a negative value (typically -1) otherwise
 */
int8_t pic_clearSoftwareInterrupt(void)
{
    return pic_clearSwInterruptNr(BSP_SOFTWARE_IRQ);
}
#endif
