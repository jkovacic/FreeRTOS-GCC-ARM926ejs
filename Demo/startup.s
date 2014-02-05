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
 * Definition of exception vectors, implementation of reset (and startup handler),
 * Preparation of the IRQ mode.
 *
 * For more details, see:
 * ARM9EJ-S Technical Reference Manual (DDI0222):
 * http://infocenter.arm.com/help/topic/com.arm.doc.ddi0222b/DDI0222.pdf
 *
 * These articles are also useful:
 * http://balau82.wordpress.com/2012/04/15/arm926-interrupts-in-qemu/
 * http://www.embedded.com/design/mcus-processors-and-socs/4026075/Building-Bare-Metal-ARM-Systems-with-GNU-Part-2
 */

@ Constants that define various operating modes
.equ  PSR_MASK,    0x0000001F       @ CSPR bits that define operating mode

.equ  MODE_USR,    0x00000010       @ User Mode
.equ  MODE_FIQ,    0x00000011       @ FIQ Mode
.equ  MODE_IRQ,    0x00000012       @ IRQ Mode
.equ  MODE_SVC,    0x00000013       @ Supervisor Mode
.equ  MODE_ABT,    0x00000017       @ Abort Mode
.equ  MODE_UND,    0x0000001B       @ Undefined Mode
.equ  MODE_SYS,    0x0000001F       @ System Mode

.equ FIQ_BIT,      0x00000040       @ FIQ exception enable/disable bit
.equ IRQ_BIT,      0x00000080       @ IRQ exception enable/disable bit

.equ EXCEPTION_VECT,   0x00000000   @ Start of exception vectors


.section .init
.code 32                                   @ 32-bit ARM instruction set

@ This symbol must be visible to the linker
.global vectors_start

vectors_start:
    @ Exception vectors, relative to the base address, see page 2-26 of DDI0222
    LDR pc, reset_handler_addr             @ Reset (and startup) vector
    LDR pc, undef_handler_addr             @ Undefined (unknown) instruction
    LDR pc, swi_handler_addr               @ Software interrupt
    LDR pc, prefetch_abort_handler_addr    @ Prefetch abort
    LDR pc, data_abort_handler_addr        @ Data abort (system bus cannot access a peripheral)
    LDR pc, invalid_addr_handler           @ Reserved (early ARM only supported 26-bit addresses)
    LDR pc, irq_handler_addr               @ IRQ handler
    LDR pc, fiq_handler_addr               @ FIQ handler

@ Labels with addresses to exception handler routines, referenced above:
reset_handler_addr:
    .word reset_handler
undef_handler_addr:
    .word unhandled
swi_handler_addr:
    .word vPortYieldProcessor
prefetch_abort_handler_addr:
    .word unhandled
data_abort_handler_addr:
    .word unhandled
invalid_addr_handler:
    .word unhandled
irq_handler_addr:
    .word vFreeRTOS_ISR
fiq_handler_addr:
    .word unhandled

vectors_end:

.text
.code 32
/*
 * Implementation of the reset handler, executed also at startup.
 * It sets stack pointers for all supported operating modes (Supervisor,
 * IRQ and System), disables IRQ iand FIQ nterrupts for all modes and finally
 * it jumps into the startup function.
 *
 * Note: 'stack_top', 'irq_stack_top' and 'svc_stack_top' are allocated in qemu.ld
 */
reset_handler:
    @ The handler is always entered in Supervisor mode
    LDR sp, =svc_stack_top                 @ stack for the supervisor mode

    @  In the reset handler, we need to copy our interrupt vector table to 0x0000, its currently at 0x10000
    LDR r0, =vectors_start                 @ Store the source pointer
    MOV r1, #EXCEPTION_VECT                @ Store the destination pointer.
    @  Copy the branching instructions from vectors start to registers r2-r9 and then to destination
    LDMIA r0!, {r2, r3, r4, r5, r6, r7, r8, r9}     @ Load multiple values from indexed address; auto-increment R0
    STMIA r1!, {r2, r3, r4, r5, r6, r7, r8, r9}     @ Store multiple values from the indexed address; auto-increment R1

    @  Also copy correct addresses of exception handlers
    LDMIA r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
    STMIA r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

    @ Clear the whole BSS section to 0:
    LDR r0, __bss_begin_addr
    LDR r1, __bss_end_addr
    MOV r2, #0
bss_clear_loop:
    CMP r0, r1                     @ if (r0<r1) ....
    STRLTB r2, [r0], #1            @ ...store a byte of r2 (i.r. 0) to location pointed by r0++
    BLT bss_clear_loop             @ ...and continue the loop


    @ Set stack pointers and IRQ/FIQ bits for all supported operating modes

    MRS r0, cpsr                           @ copy Program Status Register (CPSR) to r0

    @ Disable IRQ and FIQ interrupts for the Supervisor mode
    @ This should be disabled by default, but it doesn't hurt...
    ORR r1, r0, #IRQ_BIT|FIQ_BIT
    MSR cpsr, r1

    @ Switch to System mode and disable IRQ/FIQ
    BIC r1, r0, #PSR_MASK                  @ clear lowest 5 bits
    ORR r1, r1, #MODE_SYS                  @ and set them to the System mode
    ORR r1, r1, #IRQ_BIT|FIQ_BIT           @ disable IRQ and FIW triggering
    MSR cpsr, r1                           @ update CPSR and enter System mode
    LDR sp, =stack_top                     @ set stack for System mode

    @ Set and switch into IRQ mode
    BIC r1, r0, #PSR_MASK                  @ clear least significant 5 bits...
    ORR r1, r1, #MODE_IRQ                  @ and set them to b10010 (0x12), i.e set IRQ mode
    ORR r1, r1, #IRQ_BIT|FIQ_BIT           @ also disable IRQ and FIQ triggering (a default setting, but...)
    MSR cpsr, r1                           @ update CPSR (program status register) for IRQ mode

    @ When in IRQ mode, set its stack pointer
    LDR sp, =irq_stack_top                 @ stack for the IRQ mode

    @ Prepare and enter into System mode.
    BIC r1, r0, #PSR_MASK                  @ clear lowest 5 bits
    ORR r1, r1, #MODE_SYS                  @ and set them to the System mode

    @ Return to Supervisor mode. When the first task starts it will switch
    @ to System mode and enable IRQ triggering.
    BIC r1, r1, #PSR_MASK
    ORR r1, r1, #MODE_SVC
    MSR cpsr, r1

    BL _init                               @ before the application is started, initialize all hardware

    B main                                 @ and finally start the application

unhandled:
    B .                                    @ infinite loop for unsupported exceptions

@ Addresses of BSS begin and end.
@ Note that both symbols have been defined in the linker script
__bss_begin_addr:
    .word __bss_begin
__bss_end_addr:
    .word __bss_end

.end
