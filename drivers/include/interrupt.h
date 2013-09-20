/**
 * @file
 *
 * Declaration of public functions that handle
 * the board's primary interrupt controller (PIC).
 *
 * @author Jernej Kovacic
 */

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_


#include <stdint.h>

#define PIC_MAX_PRIORITY     127

/**
 * Required prototype for vectored ISR servicing routines
 */
typedef void (*pVectoredIsrPrototype)(void);


//void irq_enableIrqMode(void);

//void irq_disableIrqMode(void);

void pic_init(void);

void pic_enableInterrupt(uint8_t irq);

void pic_disableInterrupt(uint8_t irq);

void pic_disableAllInterrupts(void);

int8_t pic_isInterruptEnabled(uint8_t irq);

int8_t pic_getInterruptType(uint8_t irq);

void pic_setInterruptType(uint8_t irq, int8_t toIrq);

void pic_setDefaultVectorAddr(pVectoredIsrPrototype addr);

int8_t pic_registerIrq(
                               uint8_t irq,
                               pVectoredIsrPrototype addr,
                               uint8_t priority );

void pic_unregisterIrq(uint8_t irq);

void pic_unregisterAllIrqs(void);

int8_t pic_setSwInterruptNr(uint8_t irq);

int8_t pic_clearSwInterruptNr(uint8_t irq);

int8_t pic_setSoftwareInterrupt(void);

int8_t pic_clearSoftwareInterrupt(void);


#endif  /* _INTERRUPT_H_ */
