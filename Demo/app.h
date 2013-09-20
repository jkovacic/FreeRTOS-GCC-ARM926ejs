/**
 * @file
 * A header file that should be included into every application's source file.
 * It provides some handy definitions, e.g. for outputting messages
 * to a UART or for creating FreeRTOS tasks with proper error handling.
 *
 * @author Jernej Kovacic
 */

#ifndef _APP_H_
#define _APP_H_

#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include "../drivers/include/uart.h"


/* Outputs a msg to the UART0 */
#define vPrintMsg(msg)         uart_print(0, (msg))

/* Creates a FreeRTOS task and handles possible errors */
#define assertTaskCreate(code, name, stackDepth, params, priority, taskHandle) \
 if ( pdPASS!=xTaskCreate( (code), (name), (stackDepth), (params), (priority), (taskHandle) ) ) \
 { \
     vPrintMsg("Could not create '"); \
     vPrintMsg(name); \
     vPrintMsg("'\r\n"); \
 }


#endif  /* _APP_H_ */
