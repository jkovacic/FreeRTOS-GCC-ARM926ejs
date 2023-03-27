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
 * A simple demo application.
 *
 * @author Jernej Kovacic
 */


#include <stddef.h>
#if USE_NEWLIB == 1
#include <stdio.h>
#include <malloc.h>
#endif

#include <FreeRTOS.h>
#include <task.h>

#include "app_config.h"
#include "bsp.h"
#include "print.h"
#include "receive.h"


#if USE_DEBUG_FLAGS == 1
void vAssertCalled( const char *pcFile, uint32_t ulLine )
{
    volatile unsigned long looping = 0UL;
#if 1
    ( void ) pcFile;
    ( void ) ulLine;
#else
    printf("Assertion failed at %s, line %d\n\r", pcFile, ulLine);
#endif
    taskENTER_CRITICAL();
    {
        /* Use the debugger to set ul to a non-zero value in order to step out
           of this function to determine why it was called. */
        while( looping == 0UL )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
     * size of the    heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might be
     * fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */

#if 1
    vAssertCalled( __FILE__, __LINE__ );
#else
    /* configPRINT_STRING(( "ERROR: Malloc failed to allocate memory\r\n" )); */
    taskDISABLE_INTERRUPTS();
    for(;;){}
#endif
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName)
{
    (void) xTask;
    (void) pcTaskName;

    /* Run time stack overflow checking is performed if
     * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     * function is called if a stack overflow is detected.  This function is
     * provided as an example only as stack overflow checking does not function
     * when running the FreeRTOS POSIX port. */
    vAssertCalled( __FILE__, __LINE__ );
}

#if USE_NEWLIB == 0
static char* itoa(int i, char b[])
{
    char *p = b;
    int shifter;

    if (i == 0) {
        *p++ = '0';
        *p = '\0';
	return b;
    }
    if (i < 0) {
        *p++ = '-';
        i *= -1;
    }
    shifter = i;
    do {
        ++p;
        shifter = shifter / 10;
    } while (shifter);
    *p = '\0';
    do {
        *--p = (char)('0' + (i % 10));
        i = i / 10;
    } while(i);
    return b;
}
#endif

void xtraceMALLOC(void *pvAddress, unsigned int uiSize)
{
#if USE_NEWLIB == 1
    char buffer[50];

    sprintf(buffer, "%d = malloc(%d)\r\n", (int)pvAddress, uiSize);
    vDirectPrintMsg(buffer);
#else
    char buffer[12];

    (void) pvAddress;
    (void) itoa((int) uiSize, buffer);
    vDirectPrintMsg("malloc(");
    vDirectPrintMsg(buffer);
    vDirectPrintMsg(") called\r\n");
#endif
}

void xtraceFREE(void *pvAddress, unsigned int uiSize)
{
#if USE_NEWLIB == 1
    char buffer[50];

    sprintf(buffer, "free(%d, %d)\r\n", (int)pvAddress, uiSize);
    vDirectPrintMsg(buffer);
#else
    (void) pvAddress;
    (void) uiSize;
    vDirectPrintMsg("free() called\r\n");
#endif
}
#endif

#if USE_NEWLIB == 1
void __malloc_lock(struct _reent *r)
{
    (void) r;
    vTaskSuspendAll();
}

void __malloc_unlock(struct _reent *r)
{
    (void) r;
    xTaskResumeAll();
}
#endif


/* Struct with settings for each task */
typedef struct
{
    const portCHAR* text;            /* text to be printed by the task */
    UBaseType_t delay;               /* delay in milliseconds */
} paramStruct;

/* Default parameters if no parameter struct is available */
static const portCHAR defaultText[] = "<NO TEXT>\r\n";
static const UBaseType_t defaultDelay = 1000U;


/* Task function - may be instantiated in multiple tasks */
static void vTaskFunction( void *pvParameters )
{
    const portCHAR* taskName;
    UBaseType_t delay;
    paramStruct* params = (paramStruct*) pvParameters;

    taskName = ( NULL==params || NULL==params->text ) ? defaultText : params->text;
    delay = ( NULL==params ) ? defaultDelay : params->delay;

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


/* Fixed frequency periodic task function - may be instantiated in multiple tasks */
static void vPeriodicTaskFunction(void* pvParameters)
{
    const portCHAR* taskName;
    UBaseType_t delay;
    paramStruct* params = (paramStruct*) pvParameters;
    TickType_t lastWakeTime;

    taskName = ( NULL==params || NULL==params->text ) ? defaultText : params->text;
    delay = ( NULL==params ) ? defaultDelay : params->delay;

    /*
     * This variable must be initialized once.
     * Then it will be updated automatically by vTaskDelayUntil().
     */
    lastWakeTime = xTaskGetTickCount();

    for( ; ; )
    {
        /* Print out the name of this task. */

        vPrintMsg(taskName);

        /*
         * The task will unblock exactly after 'delay' milliseconds (actually
         * after the appropriate number of ticks), relative from the moment
         * it was last unblocked.
         */
        vTaskDelayUntil( &lastWakeTime, delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}


/*
 * A convenience function that is called when a FreeRTOS API call fails
 * and a program cannot continue. It prints a message (if provided) and
 * ends in an infinite loop.
 */
static void FreeRTOS_Error(const portCHAR* msg)
{
    if ( NULL != msg )
    {
        vDirectPrintMsg(msg);
    }

    for ( ; ; );
}


/* Startup function that creates and runs two FreeRTOS tasks */
int main(void)
{
    /* Parameters for two tasks */
    static paramStruct tParam[2] =
    {
        { "Task1\r\n", 2000U },
        { "Periodic task\r\n", 3000U }
    };

    hw_init();

    /* Init of print related tasks: */
    if ( pdFAIL == printInit() )
    {
        FreeRTOS_Error("Initialization of print failed\r\n");
    }

    /*
     * I M P O R T A N T :
     * Make sure (in startup.s) that main is entered in Supervisor mode.
     * When vTaskStartScheduler launches the first task, it will switch
     * to System mode and enable interrupt exceptions.
     */
    vDirectPrintMsg("= = = T E S T   S T A R T E D = = =\r\n\r\n");

    /* Init of receiver related tasks: */
    if ( pdFAIL == recvInit() )
    {
        FreeRTOS_Error("Initialization of receiver failed\r\n");
    }

    /* Create a print gate keeper task: */
    if ( pdPASS != xTaskCreate(printGateKeeperTask, "gk", 128, NULL,
                               PRIOR_PRINT_GATEKEEPR, NULL) )
    {
        FreeRTOS_Error("Could not create a print gate keeper task\r\n");
    }

    if ( pdPASS != xTaskCreate(recvTask, "recv", 128, NULL, PRIOR_RECEIVER, NULL) )
    {
        FreeRTOS_Error("Could not create a receiver task\r\n");
    }

    /* And finally create two tasks: */
    if ( pdPASS != xTaskCreate(vTaskFunction, "task1", 128, (void * const) &tParam[0],
                               PRIOR_PERIODIC, NULL) )
    {
        FreeRTOS_Error("Could not create task1\r\n");
    }

    if ( pdPASS != xTaskCreate(vPeriodicTaskFunction, "task2", 128, (void * const) &tParam[1],
                               PRIOR_FIX_FREQ_PERIODIC, NULL) )
    {
        FreeRTOS_Error("Could not create task2\r\n");
    }

    vDirectPrintMsg("A text may be entered using a keyboard.\r\n");
    vDirectPrintMsg("It will be displayed when 'Enter' is pressed.\r\n\r\n");

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /*
     * If all goes well, vTaskStartScheduler should never return.
     * If it does return, typically not enough heap memory is reserved.
     */

    FreeRTOS_Error("Could not start the scheduler!!!\r\n");

    /* just in case if an infinite loop is somehow omitted in FreeRTOS_Error */
    for ( ; ; );
    return 0;
}
