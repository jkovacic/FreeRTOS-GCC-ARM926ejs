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

#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "app_config.h"
#include "bsp.h"
#include "uart.h"
#include "interrupt.h"

#include "print.h"


/* Numeric codes for special keys: */

/* This code is received when BackSpace is pressed: */
#define CODE_BS             ( 0x7F )
/* Enter (CR): */
#define CODE_CR             ( 0x0D )


/* This string is displayed first when Enter is pressed: */
#define MSG_TEXT            "You entered: \""
/* Hardcoded strlen(MSG_TEXT): */
#define MSG_OFFSET          ( 14 )
/*
 * Total length of a string buffer:
 * MSG_OFFSET + RECV_BUFFER_SIZE + additional 4 characters for "\"\r\n\0"
 */
#define RECV_TOTAL_BUFFER_LEN        ( MSG_OFFSET + RECV_BUFFER_LEN + 3 + 1 )

/* Allocated "circular" buffer */
static portCHAR buf[ RECV_BUFFER_SIZE ][ RECV_TOTAL_BUFFER_LEN ];

/* Position of the currently available slot in the buffer */
static uint16_t bufCntr = 0;

/* Position of the current character within the buffer */
static uint16_t bufPos = 0;

/* UART number: */
static uint8_t recvUartNr = ( uint8_t ) -1;

/* A queue for received characters, not processed yet */
static QueueHandle_t recvQueue;


/* forward declaration of an ISR handler: */
static void recvIsrHandler(void);


/**
 * Initializes all receive related tasks and synchronization primitives.
 * This function must be called before anything is attempted to be received!
 *
 * @param uart_nr - number of the UART
 *
 * @return pdPASS if initialization is successful, pdFAIL otherwise
 */
int16_t recvInit(uint8_t uart_nr)
{
    /* Obtain the UART's IRQ from BSP */
    const uint8_t uartIrqs[BSP_NR_UARTS] = BSP_UART_IRQS;
    const uint8_t irq = ( uart_nr<BSP_NR_UARTS ?
                             uartIrqs[uart_nr] :
                             (uint8_t) -1 );
    uint16_t i;

    for ( i=0; i<RECV_BUFFER_SIZE; ++i )
    {
        memset((void*) buf[i], '\0', RECV_TOTAL_BUFFER_LEN);
        strcpy(buf[i], MSG_TEXT);
    }

    bufCntr = 0;
    bufPos = 0;

    /* Check if UART number is valid */
    if ( uart_nr >= BSP_NR_UARTS )
    {
        return pdFAIL;
    }

    recvUartNr = uart_nr;

    /* Create and assert a queue for received characters */
    recvQueue = xQueueCreate(RECV_QUEUE_SIZE, sizeof(portCHAR));
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


/*
 * ISR handler, triggered by IRQ 12.
 * It reads a character from the UART and pushes it into the queue.
 */
static void recvIsrHandler(void)
{
    portCHAR ch;

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
 * A FreeRTOS task that processes received characters.
 * The task is waiting in blocked state until the ISR handler pushes something
 * into the queue. If the received character is valid, it will be appended to a
 * string buffer. When 'Enter' is pressed, the entire string will be sent to UART0.
 *
 * @param params - ignored
 */
void recvTask(void* params)
{
    portCHAR ch;

    for ( ; ; )
    {
        /* The task is blocked until something appears in the queue */
        xQueueReceive(recvQueue, (void*) &ch, portMAX_DELAY);

        /*
         * Although a bit long, 'switch' offers a convenient way to
         * insert or remove valid characters.
         */
        switch (ch)
        {
            /* "Ordinary" valid characters that will be appended to a buffer */

            /* Uppercase letters 'A' .. 'Z': */
            case 'A' :
            case 'B' :
            case 'C' :
            case 'D' :
            case 'E' :
            case 'F' :
            case 'G' :
            case 'H' :
            case 'I' :
            case 'J' :
            case 'K' :
            case 'L' :
            case 'M' :
            case 'N' :
            case 'O' :
            case 'P' :
            case 'Q' :
            case 'R' :
            case 'S' :
            case 'T' :
            case 'U' :
            case 'V' :
            case 'W' :
            case 'X' :
            case 'Y' :
            case 'Z' :

            /* Lowercase letters 'a'..'z': */
            case 'a' :
            case 'b' :
            case 'c' :
            case 'd' :
            case 'e' :
            case 'f' :
            case 'g' :
            case 'h' :
            case 'i' :
            case 'j' :
            case 'k' :
            case 'l' :
            case 'm' :
            case 'n' :
            case 'o' :
            case 'p' :
            case 'q' :
            case 'r' :
            case 's' :
            case 't' :
            case 'u' :
            case 'v' :
            case 'w' :
            case 'x' :
            case 'y' :
            case 'z' :

            /* Decimal digits '0'..'9': */
            case '0' :
            case '1' :
            case '2' :
            case '3' :
            case '4' :
            case '5' :
            case '6' :
            case '7' :
            case '8' :
            case '9' :

            /* Other valid characters: */
            case ' ' :
            case '_' :
            case '+' :
            case '-' :
            case '/' :
            case '.' :
            case ',' :
            {
                if ( bufPos < RECV_BUFFER_LEN )
                {
                    /* If the buffer is not full yet, append the character */
                    buf[bufCntr][MSG_OFFSET + bufPos] = ch;
                    /* and increase the position index: */
                    ++bufPos;
                }

                break;
            }

            /* Backspace must be handled separately: */
            case CODE_BS :
            {
                /*
                 * If the buffer is not empty, decrease the position index,
                 * i.e. "delete" the last character
                 */
                if ( bufPos>0 )
                {
                    --bufPos;
                }

                break;
            }

            /* 'Enter' a.k.a. Carriage Return (CR): */
            case CODE_CR :
            {
                /* Append characters to terminate the string:*/
                bufPos += MSG_OFFSET;
                buf[bufCntr][bufPos++] = '"';
                buf[bufCntr][bufPos++] = '\r';
                buf[bufCntr][bufPos++] = '\n';
                buf[bufCntr][bufPos]   = '\0';
                /* Send the entire string to the print queue */
                vPrintMsg(buf[bufCntr]);
                /* And switch to the next line of the "circular" buffer */
                ++bufCntr;
                bufCntr %= RECV_BUFFER_SIZE;
                /* "Reset" the position index */
                bufPos = 0;

                break;
            }

        }  /* switch */

    }  /* for */

    /* if it ever breaks out of the infinite loop... */
    vTaskDelete(NULL);

    /* suppress a warning since 'params' is ignored */
    (void) params;
}
