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
 *
 * A linker script that defines the application's memory map, starting point
 * and stack memory sizes.
 *
 * This script is tailored for startup in Qemu.
 *
 * To verify whether objects (symbols) have been placed to expected locations, run
 * 'nm' ('arm-none-eabi-nm') on the resulting ELF image.
 *
 * Based on these articles:
 * http://balau82.wordpress.com/2012/04/15/arm926-interrupts-in-qemu/
 * http://www.embedded.com/design/mcus-processors-and-socs/4026080/Building-Bare-Metal-ARM-Systems-with-GNU-Part-3
 */


/*
 * Memory region between 0x00000000 and 0x07FFFFFF is reserved for max. 128MB of RAM.
 * See page 4-3 of the DUI0225D.
 */

ENTRY(vectors_start)

SECTIONS
{
    /*
     * Linker will place all sections after the __ld_Init_Addr, so with an exception of
     * a very small region for exception vectors, the first 64 kB would remain unused. This
     * is a convenient region to be allocated for various operating modes' stacks. The region
     * is large enough to allow very generous stack sizes, much larger than necessary for the
     * simple test application. Should any operating mode require more memory for its stack, it
     * should be placed at the end of the SECTIONS command.
     */

     /* Constants used within this command: */
    __ld_Vectors_Size = 16 * 4;   /* Vectors at the very beginning of the memory (64 bytes) */
    __ld_Svc_Stack_Size = 0x1000; /* Very generous size of the Supervisor mode's stack (4 kB) */
    __ld_Irq_Stack_size = 0x1000; /* Very generous size of the IRQ mode's stack (4 kB) */
    __ld_Init_Addr = 0x10000;     /* Qemu starts execution at this address, 64 kB */
    __ld_Ram_size = 128M;         /* Total capacity of RAM */

    . = __ld_Vectors_Size;        /* Move the pointer after the "reserved" area for exception vectors */

    . = . + __ld_Svc_Stack_Size;  /* Allocate memory for Supervisor mode's stack */
    svc_stack_top = .;            /* Initial stack pointer for the Supervisor mode */

    . = . + __ld_Irq_Stack_size; /* Allocate memory for IRQ mode's stack */
    irq_stack_top = .;           /* Initial stack pointer for the IRQ mode */

    /* System mode's stack: 64 kB - 8 kB - 64 B = approx. 56 kB */
    . = __ld_Init_Addr - 4;      /* Allocate memory for System mode's stack */
    stack_top = .;               /* It starts just in front of the startup address */

    . = __ld_Init_Addr;          /* Qemu will boot from this address */
    .init : {
        *(.init)                 /* ARM exception vectors must be placed to startup address */
    }
    .text : {
        *(.text)
    }
    .rodata : {
        *(.rodata)
    }
    .data : {
        *(.data)
    }
    . = ALIGN(4);
    .bss : {
        __bss_start = .;
        *(.bss)
    }
    . = ALIGN(4);
    end = .;
    /* The remainder of RAM is reserved for the heap (if malloc is ever used) */
    .heap : {
        *(.heap)
        . = __ld_Ram_size - end;
    }
}
