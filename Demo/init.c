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
 * HW initialization routines that should be called before startup section
 * calls main(). Functions from this file should not be publicly exposed
 * in headers.
 *
 * @author Jernej Kovacic
 */

 #include <stdint.h>

 #include "bsp.h"

 #include "interrupt.h"
 #include "timer.h"
 #include "uart.h"

 /*
  * Performs initialization of all supported hardware.
  * All peripherals are stopped, their interrupt triggering is disabled, etc.
  */
 void _init(void)
 {
     uint8_t i;
     uint8_t j;
     const uint8_t ctrs = timer_countersPerTimer();

     /* Disable IRQ triggering (may be reenabled after ISRs are properly set) */
     irq_disableIrqMode();

     /* Init the vectored interrupt controller */
     pic_init();

     /* Init all counters of all available timers */
     for ( i=0; i<BSP_NR_TIMERS; ++i )
     {
         for ( j=0; j<ctrs; ++j )
         {
             timer_init(i, j);
         }
     }

     /* Init all available UARTs */
     for ( i=0; i<BSP_NR_UARTS; ++i )
     {
         uart_init(i);
     }

}
