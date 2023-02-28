# Copyright 2013, 2017, Jernej Kovacic
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software. If you wish to use our Amazon
# FreeRTOS name, please do so in a fair use way that does not cause confusion.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#
# Type "make help" for more details.
#


#
# Do we want to link against newlib?
#
USE_NEWLIB=0

#
# Compile a debugging version?
#
USE_DEBUG_FLAGS=0


# Version "6-2017-q2-update" of the "GNU Arm Embedded Toolchain" is used
# as a build tool. See comments in "setenv.sh" for more details about
# downloading it and setting the appropriate environment variables.

TOOLCHAIN = arm-none-eabi-
CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCOPY = $(TOOLCHAIN)objcopy
AR = $(TOOLCHAIN)ar

# GCC flags
CFLAG = -c
OFLAG = -o
INCLUDEFLAG = -I
CPUFLAG = -mcpu=arm926ej-s
WFLAG = -Wall -Wextra -pedantic
#WFLAG += -Werror
#WFLAG += -Wundef -Wshadow -Wwrite-strings -Wold-style-definition -Wcast-align=strict -Wunreachable-code -Waggregate-return -Wlogical-op -Wtrampolines -Wc90-c99-compat -Wc99-c11-compat
#WFLAG += -Wconversion -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Wcast-qual -Wswitch-default
CFLAGS = $(CPUFLAG) $(WFLAG) -O2

ifeq ($(USE_NEWLIB),1)
CFLAGS += --specs=nano.specs --specs=nosys.specs -DUSE_NEWLIB=1
else
CFLAGS += -DUSE_NEWLIB=0
endif

ifeq ($(USE_DEBUG_FLAGS),1)
# Additional C compiler flags to produce debugging symbols
CFLAGS += -g -DDEBUG -DUSE_DEBUG_FLAGS___XXX=1
else
CFLAGS += -g -DUSE_DEBUG_FLAGS=0
endif


# Compiler/target path in FreeRTOS/Source/portable
PORT_COMP_TARG = GCC/ARM926EJ-S

# Intermediate directory for all *.o and other files:
OBJDIR = obj/

# FreeRTOS source base directory
FREERTOS_SRC = FreeRTOS/Source

# Directory with memory management source files
FREERTOS_MEMMANG_SRC = $(FREERTOS_SRC)/portable/MemMang

# Directory with platform specific source files
FREERTOS_PORT_SRC = $(FREERTOS_SRC)/portable/$(PORT_COMP_TARG)

# Directory with HW drivers' source files
DRIVERS_SRC = drivers

# Directory with demo specific source (and header) files
APP_SRC = Demo


# Object files to be linked into an application
# Due to a large number, the .o files are arranged into logical groups:

FREERTOS_OBJS = queue.o list.o tasks.o
# The following o. files are only necessary if
# certain options are enabled in FreeRTOSConfig.h
#FREERTOS_OBJS += timers.o
#FREERTOS_OBJS += croutine.o
#FREERTOS_OBJS += event_groups.o
#FREERTOS_OBJS += stream_buffer.o

# Only one memory management .o file must be uncommented!
ifeq ($(USE_NEWLIB),0)
FREERTOS_MEMMANG_OBJS = heap_1.o
endif
#FREERTOS_MEMMANG_OBJS = heap_2.o
ifeq ($(USE_NEWLIB),1)
FREERTOS_MEMMANG_OBJS = heap_3.o
endif
#FREERTOS_MEMMANG_OBJS = heap_4.o
#FREERTOS_MEMMANG_OBJS = heap_5.o

FREERTOS_PORT_OBJS = port.o portISR.o
STARTUP_OBJ = startup.o
DRIVERS_OBJS = timer.o interrupt.o uart.o

APP_OBJS = init.o main.o print.o receive.o
ifeq ($(USE_NEWLIB),0)
APP_OBJS += nostdlib.o
endif


# All object files specified above are prefixed the intermediate directory
OBJS = $(addprefix $(OBJDIR), $(STARTUP_OBJ) $(FREERTOS_OBJS) $(FREERTOS_MEMMANG_OBJS) $(FREERTOS_PORT_OBJS) $(DRIVERS_OBJS) $(APP_OBJS))

# Definition of the linker script and final targets
LINKER_SCRIPT = $(addprefix $(APP_SRC)/, qemu.ld)
ELF_IMAGE = image.elf
MAPFILE = image.map
TARGET = image.bin

# Include paths to be passed to $(CC) where necessary
INC_FREERTOS = $(FREERTOS_SRC)/include
INC_DRIVERS = $(DRIVERS_SRC)

# Complete include flags to be passed to $(CC) where necessary
INC_FLAGS = $(INCLUDEFLAG)$(INC_FREERTOS) $(INCLUDEFLAG)$(APP_SRC) $(INCLUDEFLAG)$(FREERTOS_PORT_SRC)
INC_FLAG_DRIVERS = $(INCLUDEFLAG)$(INC_DRIVERS)

# Dependency on HW specific settings
DEP_BSP = $(INC_DRIVERS)/bsp.h


#
# Make rules:
#

all : $(TARGET)

rebuild : clean all

$(TARGET) : $(OBJDIR) $(ELF_IMAGE)
	$(OBJCOPY) -O binary $(word 2,$^) $@

$(OBJDIR) :
	mkdir -p $@

$(ELF_IMAGE) : $(OBJS) $(LINKER_SCRIPT)
ifeq ($(USE_NEWLIB),0)
	$(CC) -nostdlib -g -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@ -Wl,-Map=$(MAPFILE)
else
	$(CC) --specs=nano.specs --specs=nosys.specs -g -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@ -Wl,-Map=$(MAPFILE)
endif


# Startup code, implemented in assembler

$(OBJDIR)startup.o : $(APP_SRC)/startup.s
	$(AS) $(CPUFLAG) $< $(OFLAG) $@


# FreeRTOS core
$(OBJDIR)%.o : $(FREERTOS_SRC)/%.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@


# HW specific part, in FreeRTOS/Source/portable/$(PORT_COMP_TARGET)
$(OBJDIR)%.o : $(FREERTOS_PORT_SRC)/%.c
	$(CC) $(CFLAG) $(CFLAGS) -O1 $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

# Rules for all MemMang implementations are provided
# Only one of these object files must be linked to the final target
$(OBJDIR)%.o : $(FREERTOS_MEMMANG_SRC)/%.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

# Drivers
$(OBJDIR)%.o : $(DRIVERS_SRC)/%.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@


# Demo application

$(OBJDIR)main.o : $(APP_SRC)/main.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)init.o : $(APP_SRC)/init.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)print.o : $(APP_SRC)/print.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)receive.o : $(APP_SRC)/receive.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

# Or use -ffreestanding or -fno-hosted instead?
# They all disable -fno-tree-loop-distribute-patterns.
$(OBJDIR)nostdlib.o : $(APP_SRC)/nostdlib.c
	$(CC) $(CFLAG) $(CFLAGS) -fno-builtin $< $(OFLAG) $@


# Cleanup directives:

clean_obj :
	$(RM) -r $(OBJDIR)

clean_intermediate : clean_obj
	$(RM) $(ELF_IMAGE) $(MAPFILE)

clean : clean_intermediate
	$(RM) $(TARGET)

# Short help instructions:

help :
	@echo
	@echo Valid targets:
	@echo - all: builds missing dependencies and creates the target image \'$(TARGET)\'.
	@echo - rebuild: rebuilds all dependencies and creates the target image \'$(TARGET)\'.
	@echo - clean_obj: deletes all object files, only keeps \'$(ELF_IMAGE)\' and \'$(TARGET)\'.
	@echo - clean_intermediate: deletes all intermediate binaries, only keeps the target image \'$(TARGET)\'.
	@echo - clean: deletes all intermediate binaries, incl. the target image \'$(TARGET)\'.
	@echo - help: displays these help instructions.
	@echo - run: run qemu.
	@echo


run : all
	@echo "Please exit qemu by pressing \"Ctrl-A x\""
	qemu-system-arm -M versatilepb -nographic -m 128 -kernel $(TARGET)


.PHONY : all rebuild clean clean_obj clean_intermediate help run
