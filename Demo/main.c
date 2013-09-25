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
 * A simple demo application.
 *
 * @author Jernej Kovacic
 */


#include <stddef.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include "print.h"
#include "receive.h"


/* Uart(s) to print to and/or to receive from */
#define PRINT_UART_NR          ( 0 )
#define RECV_UART_NR           ( 0 )


/* Struct with settings for each task */
typedef struct _paramStruct
{
    char* text;        /* text to be printed by the task */
    uint32_t delay;    /* delay in miliseconds */
} paramStruct;

/* Default parameters if no parameter struct is available */
static const char defaultText[] = "<NO TEXT>\r\n";
static const uint32_t defaultDelay = 1000;


/* Task function that will be instantiated in multiple tasks */
void vTaskFunction( void *pvParameters )
{
    const char* taskName;
    uint32_t delay;
    paramStruct* params = (paramStruct*) pvParameters;

    taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    for( ; ; )
    {
        /* Print out the name of this task. */

        vPrintMsg(taskName);

        vTaskDelay( delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}

/* Parameters for two tasks */
static const paramStruct tParam[2] =
{
    (paramStruct) { .text="Task1\r\n", .delay=2000 },
    (paramStruct) { .text="Task2\r\n", .delay=3000 }
};


/*
 * A convenience function that is called when a FreeRTOS API call fails
 * and a programcannot continue. It prints a messgae (if provided) and
 * ends in an infinite loop.
 */
static void FreeRTOS_Error(const char* msg)
{
    if ( NULL != msg )
    {
        vDirectPrintMsg(msg);
    }

    for ( ; ; );
}

/* Startup function that creates and runs two FreeRTOS tasks */
void main(void)
{
    /* Init of print related tasks: */
    if ( pdFAIL == printInit(PRINT_UART_NR) )
    {
        FreeRTOS_Error("Initialization of print failed\r\n");
    }

    /*
     * Make sure (instartup.s) that main is entered in Supervisor mode.
     * When vTaskStartScheduler launches the first task, it will switch
     * to System mode and enable interrupt exceptions.
     */
    vDirectPrintMsg("= = = T E S T   S T A R T E D = = =\r\n\r\n");

    /* Init of receiver related tasks: */
    if ( pdFAIL == recvInit(RECV_UART_NR) )
    {
        FreeRTOS_Error("Initialization of receiver failed\r\n");
    }

    /* Create a print gate keeper task: */
    if ( pdPASS != xTaskCreate(printGateKeeperTask, "gk", 128, NULL, 1, NULL) )
    {
        FreeRTOS_Error("Could not create a print gate keeper task\r\n");
    }

    if ( pdPASS != xTaskCreate(recvTask, "recv", 128, NULL, 1, NULL) )
    {
        FreeRTOS_Error("Could not create a receiver task\r\n");
    }

    /* And finally create two tasks: */
    if ( pdPASS != xTaskCreate(vTaskFunction, "task1", 128, (void*) &tParam[0], 3, NULL) )
    {
        FreeRTOS_Error("Could not create task1\r\n");
    }

    if ( pdPASS != xTaskCreate(vTaskFunction, "task2", 128, (void*) &tParam[1], 2, NULL) )
    {
        FreeRTOS_Error("Could not create task2\r\n");
    }

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /*
     * If all goes well, vTaskStartScheduler should never return.
     * If it does return, typically not enough heap memory is reserved.
     */

    FreeRTOS_Error("Could not start the scheduler!!!\r\n");

    /* just in case if an infinite loop is somehow omitted in FreeRTOS_Error */
    for ( ; ; );
}
