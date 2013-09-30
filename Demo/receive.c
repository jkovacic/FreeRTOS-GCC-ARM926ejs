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
 * Implementation of functions that handle data receiving via a UART.
 *
 * @author Jernej Kovacic
 */


#include <FreeRTOS.h>
#include <queue.h>
#include <string.h>

#include "bsp.h"
#include "uart.h"
#include "interrupt.h"

#include "print.h"


/*
 * Hardcoded definitions of a "circular" buffer that stores strings to be printed.
 * TODO
 * The settings could be defined in a separate header file (e.g. settings.h),
 * however, the current functionality is just a temporary "solution".
 */

/*
 * Size of a buffer holding received characters, that have not been processed yet.
 * Its optimal size depends on typing speed.
 */
#define RECV_QUEUE_SIZE       ( 10 )

/*
 * A string that will be printed when a character has been received.
 * Note: 'A' will be replaced by the actual character.
 */
#define RECV_MSG  "You pressed 'A'\r\n"

/* Size of the "circular" buffer, i.e. number ofstrings */
#define RECV_BUFFER_SIZE      ( 10 )

/* Length of RECV_MESG, including 2 characters for "\r\n" and an additional one for '\0' */
#define RECV_STRING_LEN       ( 18 )

/* Allocated "circular" buffer */
static char msgBuf[ RECV_BUFFER_SIZE ][ RECV_STRING_LEN ];

/* Position of the currently available slot in the buffer */
static unsigned portSHORT bufCntr = 0;

/* Postion within the buffer where the received character will be placed. */
#define BUF_POS               ( 13 )

/* UART number: */
static unsigned portSHORT recvUartNr = (unsigned portSHORT) -1;

/* A queue for received characters, not processed yet */
static xQueueHandle recvQueue;



/*
 * ISR handler, triggered by IRQ 12.
 * It reads a character from the UART and pushes it into the queue.
 */
static void recvIsrHandler(void)
{
    char ch;

    /* Get the received character from the UART */
    ch = uart_readChar(recvUartNr);

    /*
     * Push it to the queue.
     * Note, since this is not a FreeRTOS task,
     * a *FromISR implementation of the command must be called!
     */
    xQueueSendToBackFromISR(recvQueue, (void*) &ch, 0);
    /* And acknowledge the interrupt on the UART controller */
    uart_clearRxInterrupt(recvUartNr);
}


/**
 * Initializes all receive related tasks and synchronization primitives.
 * This function must be called before anything is attempted to be received!
 *
 * @param uart_nr - number of the UART
 *
 * @return pdPASS if initialization is successful, pdFAIL otherwise
 */
portSHORT recvInit(unsigned portSHORT uart_nr)
{
    /* Obtain the UART's IRQ from BSP */
    const unsigned portSHORT uartIrqs[BSP_NR_UARTS] = BSP_UART_IRQS;
    const unsigned portSHORT irq = ( uart_nr<BSP_NR_UARTS ?
                                     uartIrqs[uart_nr] :
                                     (unsigned portSHORT) -1 );

    const char msg[] = RECV_MSG;
    unsigned portSHORT i;

    /* Initialize the "circular" buffer */
    for ( i=0; i<RECV_BUFFER_SIZE; ++i )
    {
        strcpy(msgBuf[i], msg);
    }  /* for i */

    bufCntr = 0;

    /* Check if UART number is valid */
    if ( uart_nr >= BSP_NR_UARTS )
    {
        return pdFAIL;
    }

    recvUartNr = uart_nr;

    /* Create and assert a queue for received characters */
    recvQueue = xQueueCreate(RECV_QUEUE_SIZE, sizeof(char));
    if ( 0 == recvQueue )
    {
        return pdFAIL;
    }

    /* Attempt to register UART's IRQ on VIC */
    if ( pic_registerIrq(irq, &recvIsrHandler, 50) < 0 )
    {
        return pdFAIL;
    }

    /* Enable the UART's IRQ on VIC */
    pic_enableInterrupt(irq);

    /* Configure the UART to receive data and trigger interrupts on receive */
    uart_enableRx(recvUartNr);
    uart_enableRxInterrupt(recvUartNr);

    return pdPASS;
}


/**
 * A FreeRTOS task that processes received characters.
 * The task is waitng in blocked state until the ISR handler pushes something
 * into the queue. Then the received character is displayed.
 *
 * @param params - ignored
 */
void recvTask(void* params)
{
    char ch;

    for ( ; ; )
    {
        /* The task is blocked until something appears in the queue */
        xQueueReceive(recvQueue, (void*) &ch, portMAX_DELAY);

        /* Update the current element of the buffer... */
        msgBuf[bufCntr][BUF_POS] = ch;

        /* Send the buffer's element tothe print queue... */
        vPrintMsg(msgBuf[bufCntr]);

        /* And update the "pointer" of the "circular" buffer. */
        ++bufCntr;
        bufCntr %= RECV_BUFFER_SIZE;
    }

    /* if it ever breaks out of the infinite loop... */
    vTaskDelete(NULL);
}
