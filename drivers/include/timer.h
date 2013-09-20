/**
 * @file
 *
 * Declaration of public functions that handle
 * the board's timer controller.
 *
 * @author Jernej Kovacic
 */


#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>


void timer_init(uint8_t timerNr, uint8_t counterNr);

void timer_start(uint8_t timerNr, uint8_t counterNr);

void timer_stop(uint8_t timerNr, uint8_t counterNr);

int8_t timer_isEnabled(uint8_t timerNr, uint8_t counterNr);

void timer_enableInterrupt(uint8_t timerNr, uint8_t counterNr);

void timer_disableInterrupt(uint8_t timerNr, uint8_t counterNr);

void timer_clearInterrupt(uint8_t timerNr, uint8_t counterNr);

void timer_setLoad(uint8_t timerNr, uint8_t counterNr, uint32_t value);

uint32_t timer_getValue(uint8_t timerNr, uint8_t counterNr);

const volatile uint32_t* timer_getValueAddr(uint8_t timerNr, uint8_t counterNr);

uint8_t timer_countersPerTimer(void);

#endif  /* _TIMER_H_*/
