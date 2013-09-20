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

     /* Init the vectore interrupt controller */
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
