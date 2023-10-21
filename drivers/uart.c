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
 * Implementation of the board's UART functionality.
 * All 3 UARTs are supported.
 *
 * More info about the board and the UART controller:
 * - Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 * - PrimeCell UART (PL011) Technical Reference Manual (DDI0183):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183f/DDI0183.pdf
 *
 * @author Jernej Kovacic
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "regutil.h"
#include "bsp.h"
#include "uart.h"


/*
 * Bit masks for the Control Register (UARTCR).
 *
 * For a detailed description of each control register's bit, see page 3-15 of DDI0183:
 *
 *   0: UARTEN (enable bit):  0 disabled; 1 enabled
 *   1: SIREN
 *   2: SIRLP (IrDA SIR low power mode)
 * 3-6: reserved (do not modify)
 *   7: LBE (loopback enabled)
 *   8: TXE (transmit enable): 0 disabled; 1 enabled
 *   9: RXE (receive enable): 0 disabled; 1 enabled
 *  10: DTR (data transmit ready)
 *  11: RTS (request to send)
 *  12: Out1
 *  13: Out2
 *  14: RTSEn (RTS hardware flow control enable)
 *  15: CTSEn (CTS hardware flow control enable)
 *  16-31: reserved (do not modify)
 */
#define CTL_UARTEN     ( 0x00000001U )
#define CTL_SIREN      ( 0x00000002U )
#define CTL_SIRLP      ( 0x00000004U )
#define CTL_LBE        ( 0x00000080U )
#define CTL_TXE        ( 0x00000100U )
#define CTL_RXE        ( 0x00000200U )
#define CTL_DTR        ( 0x00000400U )
#define CTL_RTS        ( 0x00000800U )
#define CTL_OUT1       ( 0x00001000U )
#define CTL_OUT2       ( 0x00002000U )
#define CTL_RTSEn      ( 0x00004000U )
#define CTL_CTSEn      ( 0x00008000U )


/*
 * Bit masks for the IMSC (Interrupt Mask Set/Clear) register.
 *
 * For a detailed description of each IMSC's bit, see page 3-18 of DDI0183:
 *    0: nUARTRI modem interrupt mask
 *    1: nUARTCTS modem interrupt mask
 *    2: nUARTDCD modem interrupt mask
 *    3: nUARTDSR modem interrupt mask
 *    4: Receive interrupt mask
 *    5: Transmit interrupt mask
 *    6: Receive timeout interrupt mask
 *    7: Framing error interrupt mask
 *    8: Parity error interrupt mask
 *    9: Break error interrupt mask
 *   10: Overrun error interrupt mask
 * 11-31: reserved, do not modify
 */
#define INT_RIMIM      ( 0x00000001U )
#define INT_CTSMIM     ( 0x00000002U )
#define INT_DCDMIM     ( 0x00000004U )
#define INT_DSRMIM     ( 0x00000008U )
#define INT_RXIM       ( 0x00000010U )
#define INT_TXIM       ( 0x00000020U )
#define INT_RTIM       ( 0x00000040U )
#define INT_FEIM       ( 0x00000080U )
#define INT_PEIM       ( 0x00000100U )
#define INT_BEIM       ( 0x00000200U )
#define INT_OEIM       ( 0x00000400U )


/*
 * Bitmasks for the Flag Register.
 *
 * For a detailed description of each flag register's bit, see page 3-8 of the DDI0183.
 *   0: Clear to send. This bit is the complement of the UART clear to send (nUARTCTS) modem status input.
 *   1: Data set ready. This bit is the complement of the UART data set ready (nUARTDSR) modem status input.
 *   2: Data carrier detect. This bit is the complement of the UART data carrier detect (nUARTDCD) modem status input.
 *   3: UART busy.
 *   4: Receive FIFO empty. The meaning of this bit depends on the state of the FEN bit in the UARTLCR_H register.
 *   5: Transmit FIFO full. The meaning of this bit depends on the state of the FEN bit in the UARTLCR_H register.
 *   6: Receive FIFO full. The meaning of this bit depends on the state of the FEN bit in the UARTLCR_H register.
 *   7: Transmit FIFO empty. The meaning of this bit depends on the state of the FEN bit in the UARTLCR_H register.
 *   8: Ring indicator. This bit is the complement of the UART ring indicator (nUARTRI) modem status input.
 * 9-31: reserved, do not modify
 */
#define FR_CTS         ( 0x00000001U )
#define FR_DSR         ( 0x00000002U )
#define FR_DCD         ( 0x00000004U )
#define FR_BUSY        ( 0x00000008U )
#define FR_RXFE        ( 0x00000010U )
#define FR_TXFF        ( 0x00000020U )
#define FR_RXFF        ( 0x00000040U )
#define FR_TXFE        ( 0x00000080U )
#define FR_RI          ( 0x00000100U )


/*
 * 32-bit Registers of individual UART controllers,
 * relative to the controller's base address:
 * See page 3-3 of DDI0183.
 *
 * Note: all registers are 32-bit words, however all registers only use the least
 * significant 16 bits (or even less). DDI0183 does not explicitly mention, how
 * the remaining bits should be handled, therefore they will be treated as
 * "should not be modified".
 */
typedef volatile struct
{
    uint8_t UARTDR[4];                 /* UART Data Register, UARTDR */
    uint32_t UARTRSR;                  /* Receive Status Register, Error Clear Register, UARTRSR/UARTECR */
    uint32_t Reserved1[4];             /* reserved, should not be modified */
    uint32_t UARTFR;                   /* Flag Register, UARTFR, read only */
    uint32_t Reserved2;                /* reserved, should not be modified */
    uint32_t UARTILPR;                 /* IrDA Low-Power Counter Register, UARTILPR */
    uint32_t UARTIBRD;                 /* Integer Baud Rate Register, UARTIBRD */
    uint32_t UARTFBRD;                 /* Fractional Baud Rate Register, UARTFBRD */
    uint32_t UARTLC_H;                 /* Line Control Register, UARTLC_H */
    uint32_t UARTCR;                   /* Control Register, UARTCR */
    uint32_t UARTIFLS;                 /* Interrupt FIFO Level Select Register, UARTIFLS */
    uint32_t UARTIMSC;                 /* Interrupt Mask Set/Clear Register, UARTIMSC */
    uint32_t UARTRIS;                  /* Raw Interrupt Status Register, UARTRIS, read only */
    uint32_t UARTMIS;                  /* Mask Interrupt Status Register, UARTMIS, read only */
    uint32_t UARTICR;                  /* Interrupt Clear Register */
    uint32_t UARTDMACR;                /* DMA Control Register, UARTDMACR */
    uint32_t Reserved3[13];            /* reserved, should not be modified */
    uint32_t ReservedTest[4];          /* reserved for test purposes, should not be modified */
    uint32_t Reserved4[976];           /* reserved, should not be modified */
    uint32_t ReservedIdExp[4];         /* reserved for future ID expansion, should not be modified */
    uint32_t UARTPeriphID[4];          /* UART peripheral ID, read only */
    uint32_t UARTCellID[4];            /* UART Cell ID, read only */
} ARM926EJS_UART_REGS;

/* Shared UART register: */
/* #define UARTECR       UARTRSR */


#define CAST_ADDR(ADDR)    (ARM926EJS_UART_REGS*) (ADDR),

static ARM926EJS_UART_REGS * const pReg[BSP_NR_UARTS] =
                         {
                             BSP_UART_BASE_ADDRESSES(CAST_ADDR)
                         };

#undef CAST_ADDR


#ifdef DEBUG
#define CHECK_UART 1
#endif


/**
 * Initializes a UART controller.
 * It is enabled for transmission (Tx) only, receive must be enabled separately.
 * By default all IRQ sources are disabled (masked out).
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
static void uart_init(uint8_t nr)
{
    /*
     * Registers' reserved bits should not be modified.
     * For that reason, the registers are set in two steps:
     * - the appropriate bit masks of 1-bits are bitwise or'ed to the register
     * - zero complements of the appropriate bit masks of 0-bits are bitwise and'ed to the register
     */


    /*
     * Whatever the current state, as suggested on page 3-16 of the DDI0183, the UART
     * should be disabled first:
     */
    HWREG_CLEAR_BITS( pReg[nr]->UARTCR, CTL_UARTEN );

    /* Set Control Register's TXE to 1: */
    HWREG_SET_BITS( pReg[nr]->UARTCR, CTL_TXE );

    /*
     * Set all other bits (except UARTEN) of the Control Register to 0:
     * - SIREN
     * - SIRLP
     * - LBE
     * - RXE
     * - DTR
     * - RTS
     * - Out1
     * - Out2
     * - RTSEn
     * - CTSEn
     */
    HWREG_CLEAR_BITS( pReg[nr]->UARTCR, (CTL_SIREN | CTL_SIRLP | CTL_LBE | CTL_RXE | CTL_DTR) );
    HWREG_CLEAR_BITS( pReg[nr]->UARTCR, ( CTL_RTS | CTL_OUT1 | CTL_OUT2 | CTL_RTSEn | CTL_CTSEn ) );


    /* By default, all interrupts are masked out (i.e. cleared to 0): */
    HWREG_CLEAR_BITS( pReg[nr]->UARTIMSC, ( INT_RIMIM | INT_CTSMIM | INT_DCDMIM | INT_DSRMIM | INT_RXIM | INT_TXIM ) );
    HWREG_CLEAR_BITS( pReg[nr]->UARTIMSC, ( INT_RTIM | INT_FEIM | INT_PEIM | INT_BEIM | INT_OEIM ) );

    /* TODO: line control... */

    /* Finally enable the UART: */
    HWREG_SET_BITS( pReg[nr]->UARTCR, CTL_UARTEN );

    /* reserved bits remained unmodified */
}


/**
 * Initializes a UART controller.
 */
void all_uart_init(void)
{
    uint8_t i;

    /* Init all available UARTs */
    for ( i = 0U; i < BSP_NR_UARTS; ++i )
    {
        uart_init(i);
    }
}


/*
 * Outputs a character to the specified UART. This short function is used by other functions,
 * that is why it is implemented as an inline function.
 *
 * As the function is "private", it trusts its caller functions, that 'nr'
 * is valid (between 0 and 2).
 *
 * @param nr - number of the UART (between 0 and 2)
 * @param ch - character to be sent to the UART
 */
static inline __attribute__((always_inline)) void __printCh(uint8_t nr, char ch)
{
   /*
    * Qemu ignores other UART's registers, anyway the Flag Register is checked
    * to better emulate a "real" UART controller.
    * See description of the register on page 3-8 of DDI0183 for more details.
    */

   /*
    * Poll the Flag Register's TXFF bit until the Transmit FIFO is not full.
    * When the TXFF bit is set to 1, the controller's internal Transmit FIFO is full.
    * In this case, wait until some "waiting" characters have been transmitted and
    * the TXFF is set to 0, indicating the Transmit FIFO can accept additional characters.
    */
   while ( 0U != HWREG_READ_BITS( pReg[nr]->UARTFR, FR_TXFF ) )
   {
       /* an empty loop; prevents "-Werror=misleading-indentation" */
   }

   /*
    * The Data Register is a 32-bit word, however only the least significant 8 bits
    * can be assigned the character to be sent, while other bits represent various flags
    * and should not be set to 0. For that reason, the following trick is introduced:
    *
    * Casting the Data Register's address to char* effectively turns the word into an array
    * of (four) 8-bit characters. Now, dereferencing the first character of this array affects
    * only the desired character itself, not the whole word.
    */

    pReg[nr]->UARTDR[0] = ch;
}


#if 0
/**
 * Outputs a character to the specified UART.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 * @param ch - character to be sent to the UART
 */
void uart_printChar(uint8_t nr, char ch)
{
#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    /* just use the provided inline function: */
    __printCh(nr, ch);
}
#endif


/**
 * Outputs a string to the specified UART.
 *
 * "<NULL>" is transmitted if 'str' is equal to NULL.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 * @param str - string to be sent to the UART, must be '\0' terminated.
 */
void uart_print(uint8_t nr, const char* str)
{
    /*
      if NULL is passed, avoid possible problems with dereferencing of NULL
      and print this string:
     */
    const char* null_str = "<NULL>\r\n";
    const char* cp;

#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    /* handle possible NULL value of str: */
    cp = ( NULL==str ? null_str : str );

    /*
     * Just print each character until a zero terminator is detected
     */
    for ( ; '\0' != *cp; ++cp )
    {
        __printCh(nr, *cp);
    }
}


#if 0
/**
 * Enables the specified UART controller.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_enableUart(uint8_t nr)
{
#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    HWREG_SET_BITS( pReg[nr]->UARTCR, CTL_UARTEN );
}


/**
 * Disables the specified UART controller.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_disableUart(uint8_t nr)
{
#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    HWREG_CLEAR_BITS( pReg[nr]->UARTCR, CTL_UARTEN );
}
#endif


/*
 * Sets or clears a bit of the Control Register. This function is short and
 * used by other functions, this is why it is implemented as an inline function.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 * @param set - true: bitmask's bit(s) are set to 1;  false: bits are cleared to 0
 * @param bitmask - bitmask of 1-bits that will be set or cleared
 */
static inline __attribute__((always_inline)) void __setCrBit(uint8_t nr, bool set, uint32_t bitmask)
{
    uint32_t enabled;

#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    /* Store UART's enable status (UARTEN) */
    enabled = HWREG_READ_BITS( pReg[nr]->UARTCR, CTL_UARTEN );

    /*
     * As suggested on page 3-16 of the DDI0183, the UART should be disabled
     * prior to any modification of the Control Register
     */
    HWREG_CLEAR_BITS( pReg[nr]->UARTCR, CTL_UARTEN );

    /* Depending on 'set'... */
    if (set)
    {
        /* Set bitmask's bits to 1 using bitwise OR */
        HWREG_SET_BITS( pReg[nr]->UARTCR, bitmask );
    }
    else
    {
        /* Clear bitmask's bits to 0 using bitwise AND */
        HWREG_CLEAR_BITS( pReg[nr]->UARTCR, bitmask );
    }

    /* Reenable the UART if it was been enabled before */
    if (enabled)
    {
        HWREG_SET_BITS( pReg[nr]->UARTCR, CTL_UARTEN );
    }
}


/**
 * Enables specified UART's transmit (Tx) section.
 * UART's general enable status (UARTEN) remains unmodified.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_enableTx(uint8_t nr)
{
    __setCrBit(nr, true, CTL_TXE);
}


#if 0
/**
 * Disables specified UART's transmit (Tx) section.
 * UART's general enable status (UARTEN) remains unmodified.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_disableTx(uint8_t nr)
{
    __setCrBit(nr, false, CTL_TXE);
}
#endif


/**
 * Enables specified UART's transmit (Rx) section.
 * UART's general enable status (UARTEN) remains unmodified.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_enableRx(uint8_t nr)
{
    __setCrBit(nr, true, CTL_RXE);
}


#if 0
/**
 * Disables specified UART's transmit (Rx) section.
 * UART's general enable status (UARTEN) remains unmodified.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_disableRx(uint8_t nr)
{
    __setCrBit(nr, false, CTL_RXE);
}
#endif


/**
 * Enables the interrupt triggering by the specified UART when a character is received.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_enableRxInterrupt(uint8_t nr)
{
#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    /* Set bit 4 of the IMSC register: */
    HWREG_SET_BITS( pReg[nr]->UARTIMSC, INT_RXIM );
}


#if 0
/**
 * Disables the interrupt triggering by the specified UART when a character is received.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_disableRxInterrupt(uint8_t nr)
{
#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    /* Clear bit 4 of the IMSC register: */
    HWREG_CLEAR_BITS( pReg[nr]->UARTIMSC, INT_RXIM );
}
#endif


/**
 * Clears receive interrupt at the specified UART.
 *
 * Nothing is done if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 */
void uart_clearRxInterrupt(uint8_t nr)
{
#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return;
    }
#endif

    /*
     * The register is write only, so usage of the |= operator is not permitted.
     * Anyway, zero-bits have no effect on their corresponding interrupts so it
     * is perfectly OK simply to write the appropriate bitmask to the register.
     */
    pReg[nr]->UARTICR = INT_RXIM;
}


/**
 * Reads a character that was received by the specified UART.
 * The function may block until a character appears in the UART's receive buffer.
 * It is recommended that the function is called, when the caller is sure that a
 * character has actually been received, e.g. by notification via an interrupt.
 *
 * A zero is returned immediately if 'nr' is invalid (equal or greater than 3).
 *
 * @param nr - number of the UART (between 0 and 2)
 *
 * @return character received at the UART
 */
char uart_readChar(uint8_t nr)
{
#ifdef CHECK_UART
    /* Sanity check */
    if ( nr >= BSP_NR_UARTS )
    {
        return (char) 0;
    }
#endif

    /* Wait until the receiving FIFO is not empty */
    while ( 0U != HWREG_READ_BITS( pReg[nr]->UARTFR, FR_RXFE ) );

    /*
     * UART DR is a 32-bit register and only the least significant byte must be returned.
     * Casting its address to char* effectively turns the word into an array
     * of (four) 8-bit characters. Now, dereferencing the first character of this array affects
     * only the desired character itself, not the whole word.
     */

    return pReg[nr]->UARTDR[0];
}
