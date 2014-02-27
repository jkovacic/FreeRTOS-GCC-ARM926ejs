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
 * Declaration of functions that handle printing via a UART.
 *
 * @author Jernej Kovacic
 */

#ifndef _PRINT_H_
#define _PRINT_H_

#include <FreeRTOS.h>


int16_t printInit(uint16_t uart_nr);

void printGateKeeperTask(void* params);

void vPrintMsg(const portCHAR* msg);

void vPrintChar(portCHAR ch);

void vDirectPrintMsg(const portCHAR* msg);

void vDirectPrintCh(portCHAR ch);


#endif  /* _PRINT_H_ */
