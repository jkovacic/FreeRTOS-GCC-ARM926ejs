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
