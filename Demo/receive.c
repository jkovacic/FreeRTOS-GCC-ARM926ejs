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

#include "bsp.h"
#include "uart.h"
#include "interrupt.h"

#include "print.h"


#define RECV_QUEUE_SIZE       ( 5 )


/* UART number: */
static portSHORT recvUartNr = (unsigned portSHORT) -1;

/* A que to received characters, not processed yet */
static xQueueHandle recvQueue;

/*
 * A temporary buffer for a text that will be printed
 * TODO this is just a temporary "solution"
 */
static char msgBuf[] = "You pressed 'A'\r\n";
/* Postion within the buffer where the typed character will be placed. I know, not pretty...*/
#define BUF_POS               ( 13 )


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

    /* Check if UART number is valid */
    if ( uart_nr >= BSP_NR_UARTS )
    {
        return pdFAIL;
    }

    recvUartNr = uart_nr;

    /* Create ans assert a queue for received characters */
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

        /*
         * The received character is written into the designated position
         * of the buffer string.
         * TODO this is an ugly "solution" and not thread safe! If more characters
         * appear in the queue before  previous ones are printed, the buffer text will be
         * overwritten and a message with the newest character will be displayed multiple
         * times. However, the only purpose of this task is to test the receiver's
         * functionality and another processing of input data will be introduced ASAP.
         * Till then make sure, you do not type too quickly.
         */
        msgBuf[BUF_POS] = ch;

        /* Print the string buffer */
        vPrintMsg(msgBuf);
    }

    /* if it ever breaks out of the infinite loop... */
    vTaskDelete(NULL);
}
