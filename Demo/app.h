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
