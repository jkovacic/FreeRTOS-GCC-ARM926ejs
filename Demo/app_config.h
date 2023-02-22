/*
 * Copyright 2013, 2017, Jernej Kovacic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#define PRINT_UART_NR                    ( 0U )
#define RECV_UART_NR                     ( 0U )

/*
 * Priorities of certain tasks.
 * Note: priorities should not be greater than configMAX_PRIORITIES - 1,
 * defined in FreeRTOSConfig.h (its default value equals 5).
 * If any priority is greater than this value, xTaskCreate will
 * silently reduce it.
 */
#define PRIOR_PERIODIC                   ( 2U )
#define PRIOR_FIX_FREQ_PERIODIC          ( 3U )
#define PRIOR_PRINT_GATEKEEPR            ( 1U )
#define PRIOR_RECEIVER                   ( 1U )


/* Settings for print.c */

/* Size of the queue with pointers to strings that will be printed */
#define PRINT_QUEUE_SIZE                 ( 10U )

/* Number of string buffers to print individual characters */
#define PRINT_CHR_BUF_SIZE               ( 5U )


/* Settings for receive.c */

/* Size of the queue holding received characters, that have not been processed yet. */
#define RECV_QUEUE_SIZE                  ( 10U )

/* Number of string buffers necessary to print received strings */
#define RECV_BUFFER_SIZE                 ( 3U )

/*
 * Number of characters in a buffer.
 * Note: this limit does not include '\0' and additional extra characters, necessary
 * to print the string properly.
 */
#define RECV_BUFFER_LEN                  ( 50U )

#endif  /* _APP_CONFIG_H_ */
