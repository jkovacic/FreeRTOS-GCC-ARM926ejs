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


#endif  /* _UART_H_ */
