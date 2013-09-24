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
 * Implementation of functions that perform printing messages to a UART
 *
 * @author Jernej Kovacic
 */

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#include "uart.h"

/* UART number: */
#define PRINT_UART_NR       ( 0 )

#define PRINT_QUEUE_SIZE    ( 5 )

/* Messages to be printed will be pushed to this queue */
static xQueueHandle printQueue;

/* A mutex that prevents corruption of a character buffer */
static xSemaphoreHandle printChBufMutex;

/* When individual characters are printed they must be copied to this simple buffer */
static char printChBuf[2];


/**
 * Initializes all print related tasks and synchronization primitives.
 * This function must be called before anything is attempted to be printed
 * via vPrintMsg or vPrintChar!
 *
 * @return pdPASS if initialization is successful, pdFAIL otherwise
 */
portBASE_TYPE printInit(void)
{
    /* This is not really necessary... */
    printChBuf[0] = 'A';
    /* however the second character must be assigned a string terminator */
    printChBuf[1] = '\0';

    /* Create and assert a mutex for the single character printing buffer */
    printChBufMutex = xSemaphoreCreateMutex();
    if ( NULL == printChBufMutex )
    {
        return pdFAIL;
    }

    /* Create and assert a queue for the gate keeper task */
    printQueue = xQueueCreate(PRINT_QUEUE_SIZE, sizeof(char*));
    if ( 0 == printQueue )
    {
        return pdFAIL;
    }

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
    char* message;

    for ( ; ; )
    {
        /* The task is blocked until something appears in the queue */
        xQueueReceive(printQueue, &message, portMAX_DELAY);
        /* Print the message in the queue */
        uart_print(PRINT_UART_NR, message);
    }
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
void vPrintMsg(const char* msg)
{
    if ( NULL != msg )
    {
        xQueueSendToBack(printQueue, &msg, 0);
    }
}


/**
 * Prints a character in a thread safe manner - even if the calling task preempts
 * another printing task, its message will not be corrupted. Additionally, if another
 * taskj attempts to print a character, the buffer will not be corrupted.
 *
 * @note This function may only be called when the FreeRTOS scheduler is running!
 *
 * @param ch - a character to be printed
 */
void vPrintChar(char ch)
{
    /*
     * If several tasks call this function "simultaneously", the buffer may get
     * corrupted. To prevent this, use a mutex and prevent simultaneous
     * modifications of the buffer.
     */
    xSemaphoreTake(printChBufMutex, portMAX_DELAY);

    /*
     * Put 'ch' to the first character of the buffer,
     * note that the seconfd character is '\0'.
     */
    printChBuf[0] = ch;

    /* Now the buffer may be sent to the printing queue */
    xQueueSendToBack(printQueue, printChBuf, 0);

    /* release the mutex */
    xSemaphoreGive(printChBufMutex);
}


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
void vDirectPrintMsg(const char* msg)
{
    if ( NULL != msg )
    {
        uart_print(PRINT_UART_NR, msg);
    }
}


/**
 * Prints a character directly to the UART. The function is not thread safe and
 * corruptions are possible when multiple tasks attempt to print "simultaneously".
 *
 * @note his function should only be called when the FreeRTOS scheduler is not running!
 *
 * @param ch - a character to be printed
 */
void vDirectPrintCh(char ch)
{
    uart_printChar(PRINT_UART_NR, ch);
}
