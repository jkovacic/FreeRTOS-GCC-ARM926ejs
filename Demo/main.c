/**
 * @file
 * A simple demo application.
 *
 * @author Jernej Kovacic
 */

#include <FreeRTOS.h>
#include <task.h>

#include "app.h"

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

    taskName = ( NULL==params ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    for( ; ; )
    {
        /* Print out the name of this task. */

        vPrintMsg(taskName);

        vTaskDelay( delay / portTICK_RATE_MS );
    }
}

/* Parameters for two tasks */
static const paramStruct tParam[2] =
{
    (paramStruct) { .text="Task1\r\n", .delay=2000 },
    (paramStruct) { .text="Task2\r\n", .delay=3000 }
};



/* Startup function that creates and runs two FreeRTOS tasks */
void main(void)
{
    /*
     * Make sure (instartup.s) that main is entered in Supervisor mode.
     * When vTaskStartScheduler launches the first task, it will switch
     * to System mode and enable interrupt exceptions.
     */
    vPrintMsg("= = = T E S T   S T A R T E D = = =\r\n\r\n");

    assertTaskCreate(vTaskFunction, "task1", 128, (void*) &tParam[0], 3, NULL);
    assertTaskCreate(vTaskFunction, "task2", 128, (void*) &tParam[1], 2, NULL);

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /*
     * vTaskStartScheduler should never return
     * but just in case "end up" in an infinite loop
     */
    for ( ; ; );
}
