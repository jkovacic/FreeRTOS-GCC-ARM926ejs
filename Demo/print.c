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

#include "uart.h"

/* UART number: */
#define PRINT_UART_NR       ( 0 )

#define PRINT_QUEUE_SIZE    ( 5 )

/* Messages to be printed will be pushed to this queue */
static xQueueHandle printQueue;



/**
 * Initializes all print related tasks and synchronization primitives.
 * This function must be called before anything is attempted to be printed
 * via vPrintMsg or vPrintChar!
 *
 * @return pdPASS if initialization is successful, pdFAIL otherwise
 */
portBASE_TYPE printInit(void)
{
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
