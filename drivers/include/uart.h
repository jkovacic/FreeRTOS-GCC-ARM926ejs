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
 *
 * Declaration of public functions that handle
 * the board's UART controllers.
 *
 * @author Jernej Kovacic
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>


void uart_init(uint8_t nr);

void uart_printChar(uint8_t nr, char ch);

void uart_print(uint8_t nr, const char* str);

void uart_enableUart(uint8_t nr);

void uart_disableUart(uint8_t nr);

void uart_enableTx(uint8_t nr);

void uart_disableTx(uint8_t nr);

void uart_enableRx(uint8_t nr);

void uart_disableRx(uint8_t nr);

void uart_enableRxInterrupt(uint8_t nr);

void uart_disableRxInterrupt(uint8_t nr);

void uart_clearRxInterrupt(uint8_t nr);

char uart_readChar(uint8_t nr);


#endif  /* _UART_H_ */
