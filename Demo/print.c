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
 * Implementation of functions that perform printing messages to a UART
 *
 * @author Jernej Kovacic
 */

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "app_config.h"
#include "bsp.h"
#include "uart.h"
#include "print.h"



/*
 * A string buffer is necessary for printing individual characters:
 * (1) The gate keeper tasks accepts pointers to strings only. Hence a character will
 *     be placed into a short string, its first character will be the actual character,
 *     followed by '\0'.
 * (2) Corruptions must be prevented when several tasks call vPrintChar simultaneously.
 *     To accomplish this, the buffer will consist of several strings, the optimal number
 *     depends on the application.
 */

/* The number of actual strings for the buffer has been defined in "app_config.h" */

/* Length of one buffer string, one byte for the character, the other one for '\0' */
#define CHR_BUF_STRING_LEN      ( 2 )

/* Allocate the buffer for printing individual characters */
static portCHAR printChBuf[ PRINT_CHR_BUF_SIZE ][ CHR_BUF_STRING_LEN ];

/* Position of the currently available "slot" in the buffer */
static uint16_t chBufCntr = 0U;



/* UART number: */
static uint8_t printUartNr = MY_UINT8_MAX;

/* Messages to be printed will be pushed to this queue */
static QueueHandle_t printQueue;



/**
 * Initializes all print related tasks and synchronization primitives.
 * This function must be called before anything is attempted to be printed
 * via vPrintMsg or vPrintChar!
 *
 * @param uart_nr - number of the UART
 *
 * @return pdPASS if initialization is successful, pdFAIL otherwise
 */
int16_t printInit(void)
{
    uint8_t uart_nr = PRINT_UART_NR;
    uint16_t i;

    /*
     * Initialize the character print buffer.
     * It is sufficient to set each string's second character to '\0'.
     */
    for ( i = 0U; i < PRINT_CHR_BUF_SIZE; ++i )
    {
        printChBuf[i][1] = '\0';
    }

    chBufCntr = 0U;

    /* Check if UART number is valid */
    if ( uart_nr >= BSP_NR_UARTS )
    {
        return pdFAIL;
    }

    printUartNr = uart_nr;

    /* Create and assert a queue for the gate keeper task */
    printQueue = xQueueCreate(PRINT_QUEUE_SIZE, sizeof(portCHAR*));
    if ( 0 == printQueue )
    {
        return pdFAIL;
    }

    /* Enable the UART for transmission */
    uart_enableTx(printUartNr);

    return pdPASS;
}


/**
 * A gate keeper task that waits for messages to appear in the print queue and
 * prints them. This prevents corruption of printed messages if a task that
 * actually attempts to print, is preempted.
 *
 + @param params - ignored
 */
void printGateKeeperTask(void* params)
{
    portCHAR* message;

    for ( ; ; )
    {
        /* The task is blocked until something appears in the queue */
        xQueueReceive(printQueue, (void*) &message, portMAX_DELAY);
        /* Print the message in the queue */
        uart_print(printUartNr, message);
    }

    /* if it ever breaks out of the infinite loop... */
    vTaskDelete(NULL);

    /* suppress a warning since 'params' is ignored */
    (void) params;
}


/**
 * Prints a message in a thread safe manner - even if the calling task is preempted,
 * the entire message will be printed.
 *
 * Nothing is printed if 'msg' equals NULL.
 *
 * @note This function may only be called when the FreeRTOS scheduler is running!
 *
 * @param msg - a message to be printed
 */
void vPrintMsg(const portCHAR* msg)
{
    if ( NULL != msg )
    {
        xQueueSendToBack(printQueue, (void*) &msg, 0);
    }
}


#if 0
/**
 * Prints a character in a thread safe manner - even if the calling task preempts
 * another printing task, its message will not be corrupted. Additionally, if another
 * task attempts to print a character, the buffer will not be corrupted.
 *
 * @note This function may only be called when the FreeRTOS scheduler is running!
 *
 * @param ch - a character to be printed
 */
void vPrintChar(portCHAR ch)
{
    /*
     * If several tasks call this function "simultaneously", the buffer may get
     * corrupted. To prevent this, the buffer contains several strings
     */

    /*
     * Put 'ch' to the first character of the current buffer string,
     * note that the second character has been initialized to '\0'.
     */
    printChBuf[chBufCntr][0] = ch;

    /* Now the current buffer string may be sent to the printing queue */
    vPrintMsg(printChBuf[chBufCntr]);

    /*
     * Update chBufCntr and make sure it always
     * remains between 0 and CHR_PRINT_BUF_SIZE-1
     */
    ++chBufCntr;
    chBufCntr %= PRINT_CHR_BUF_SIZE;
}
#endif


/**
 * Prints a message directly to the UART. The function is not thread safe
 * and corruptions are possible when multiple tasks attempt to print "simultaneously"
 *
 * Nothing is printed if 'msg' equals NULL.
 *
 * @note This function should only be called when the FreeRTOS scheduler is not running!
 *
 * @param msg - a message to be printed
 */
void vDirectPrintMsg(const portCHAR* msg)
{
    if ( NULL != msg )
    {
        uart_print(printUartNr, msg);
    }
}


#if 0
/**
 * Prints a character directly to the UART. The function is not thread safe and
 * corruptions are possible when multiple tasks attempt to print "simultaneously".
 *
 * @note his function should only be called when the FreeRTOS scheduler is not running!
 *
 * @param ch - a character to be printed
 */
void vDirectPrintCh(portCHAR ch)
{
    uart_printChar(printUartNr, ch);
}
#endif
