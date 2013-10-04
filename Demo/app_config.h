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
 * Settings vor various application's components, e.g. for a task that
 * receives characters or for a task that prints strings to an UART.
 *
 * Note that optimal settings depend on the actual application, e.g.
 * how frequently something must be printed or received.
 *
 * @author Jernej Kovacic
 */

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_


/* Settings for main.c */

/* Uart(s) to print to and/or to receive from */
#define PRINT_UART_NR                    ( 0 )
#define RECV_UART_NR                     ( 0 )

/*
 * Priorities of certain tasks.
 * Note: priorities should not be greater than configMAX_PRIORITIES - 1,
 * defined in FreeRTOSConfig.h (its default value equals 5).
 * If any priority is greater than this value, xTasCreate will
 * silently reduce it.
 */
#define PRIOR_PERIODIC                   ( 2 )
#define PRIOR_FIX_FREQ_PERIODIC          ( 3 )
#define PRIOR_PRINT_GATEKEEPR            ( 1 )
#define PRIOR_RECEIVER                   ( 1 )


/* Settings for print.c */

/* Size of the queue with pointers to strings that will be printed */
#define PRINT_QUEUE_SIZE                 ( 10 )

/* Number of string buffers to print individual characters */
#define PRINT_CHR_BUF_SIZE               ( 5 )


/* Settings for receive.c */

/* Size of the queue holding received characters, that have not been processed yet. */
#define RECV_QUEUE_SIZE                  ( 10 )

/* Number of string buffers necessary to print received strings */
#define RECV_BUFFER_SIZE                 ( 3 )

/*
 * Number of characters in a buffer.
 * Note: this limit does not include '\0' and additional extra characters, necessary
 * to print the string properly.
 */
#define RECV_BUFFER_LEN                  ( 50 )

#endif  /* _APP_CONFIG_H_ */
