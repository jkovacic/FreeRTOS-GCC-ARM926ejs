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

#
# Remove dead code via compiler/linker flags?
#
REMOVE_DEAD_CODE=1

#
# Use gcc -flto option (link time optimization).
# This produces slightly larger code size, so disabled by default.
#
USE_LTO=0

#
# What app to compile?
#
USE_LARGE_DEMO=0


# Version "6-2017-q2-update" of the "GNU Arm Embedded Toolchain" is used
# as a build tool. See comments in "setenv.sh" for more details about
# downloading it and setting the appropriate environment variables.

TOOLCHAIN = arm-none-eabi-
CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
#LD = $(TOOLCHAIN)ld
OBJCOPY = $(TOOLCHAIN)objcopy
OBJDUMP = $(TOOLCHAIN)objdump
SIZE = $(TOOLCHAIN)size
#AR = $(TOOLCHAIN)ar

CPUFLAG = -mcpu=arm926ej-s
WFLAG = -Wall -Wextra -pedantic
#WFLAG += -Werror
#WFLAG += -Wundef -Wshadow -Wwrite-strings -Wold-style-definition -Wcast-align=strict -Wunreachable-code -Waggregate-return -Wlogical-op -Wtrampolines -Wc90-c99-compat -Wc99-c11-compat
#WFLAG += -Wconversion -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Wcast-qual -Wswitch-default
DEPFLAGS = -MMD -MP
CFLAGS = $(DEPFLAGS) $(CPUFLAG) $(WFLAG) -O2
# possible future C++ option: -fno-use-cxa-atexit
ifeq ($(USE_NEWLIB),0)
LINKER_FLAGS = -nostdlib -g
else
LINKER_FLAGS = --specs=nano.specs --specs=nosys.specs -nostartfiles -g
endif

ifeq ($(REMOVE_DEAD_CODE),1)
CFLAGS += -ffunction-sections
# "-fdata-sections" could be added as well, but often increases size
LINKER_FLAGS += -Wl,--gc-sections
# For debugging:
#LINKER_FLAGS += -Wl,--print-gc-sections
endif

ifeq ($(USE_LTO),1)
CFLAGS += -flto
LINKER_FLAGS += -flto
endif

ifeq ($(USE_NEWLIB),1)
CFLAGS += --specs=nano.specs --specs=nosys.specs -DUSE_NEWLIB=1
else
CFLAGS += -DUSE_NEWLIB=0
endif

ifeq ($(USE_DEBUG_FLAGS),1)
# Additional C compiler flags to produce debugging symbols
CFLAGS += -g -DDEBUG -DUSE_DEBUG_FLAGS=1
else
CFLAGS += -g -DUSE_DEBUG_FLAGS=0
endif

ifeq ($(USE_LARGE_DEMO),0)
CFLAGS += -DUSE_LARGE_DEMO=0
endif

# Compiler/target path in FreeRTOS/portable
PORT_COMP_TARG = GCC/ARM926EJ-S

# Intermediate directory for all *.o and other files:
OBJDIR = obj

# FreeRTOS source base directory
FREERTOS_SRC = FreeRTOS

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
DRIVERS_OBJS = timer.o interrupt.o uart.o hw_init.o
ifeq ($(USE_NEWLIB),0)
DRIVERS_OBJS += nostdlib.o
endif
APP_OBJS = main.o print.o receive.o


# All object files specified above are prefixed the intermediate directory
OBJS = $(addprefix $(OBJDIR)/, $(STARTUP_OBJ) $(FREERTOS_OBJS) $(FREERTOS_MEMMANG_OBJS) $(FREERTOS_PORT_OBJS) $(DRIVERS_OBJS) $(APP_OBJS))

# Definition of the linker script and final targets
LINKER_SCRIPT = $(APP_SRC)/qemu.ld
ELF_IMAGE = image.elf
MAPFILE = image.map
TARGET = image.bin
LISTING = image.lst

# Include paths to be passed to $(CC) where necessary
INC_FREERTOS = $(FREERTOS_SRC)/include
INC_DRIVERS = $(DRIVERS_SRC)

# Complete include flags to be passed to $(CC) where necessary
INC_FLAGS = -I$(INC_FREERTOS) -I$(APP_SRC) -I$(FREERTOS_PORT_SRC)
INC_FLAG_DRIVERS = -I$(INC_DRIVERS)


#
# Make rules:
#

# Detect Windows with two possible ways. On Linux start parallel builds:
ifeq ($(OS),Windows_NT)
else
ifeq '$(findstring ;,$(PATH))' ';'
else
CORES?=$(shell (nproc --all || sysctl -n hw.ncpu) 2>/dev/null || echo 1)
ifneq ($(CORES),1)
.PHONY: _all
_all:
	$(MAKE) all -j$(CORES)
endif
endif
endif

all : $(TARGET) $(LISTING)

rebuild : clean all

$(OBJDIR) :
	mkdir -p $@

$(OBJS) : Makefile | $(OBJDIR)

$(TARGET) : $(ELF_IMAGE)
	$(OBJCOPY) -O binary $^ $@

$(LISTING) : $(ELF_IMAGE)
	$(OBJDUMP) -d $^ > $@

$(ELF_IMAGE) : $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LINKER_FLAGS) -T $(LINKER_SCRIPT) $(OBJS) -o $@ -Wl,-Map=$(MAPFILE)
	$(SIZE) $@

-include $(wildcard $(OBJDIR)/*.d)

# Startup code, implemented in assembler
$(OBJDIR)/startup.o : $(DRIVERS_SRC)/startup.s
	$(AS) --warn $(CPUFLAG) $< -o $@

# FreeRTOS core
$(OBJDIR)/%.o : $(FREERTOS_SRC)/%.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

# HW specific part, in FreeRTOS/portable/$(PORT_COMP_TARGET)
$(OBJDIR)/%.o : $(FREERTOS_PORT_SRC)/%.c
	$(CC) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) -c $< -o $@

# Rules for all MemMang implementations are provided
# Only one of these object files must be linked to the final target
$(OBJDIR)/%.o : $(FREERTOS_MEMMANG_SRC)/%.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

# Drivers
$(OBJDIR)/%.o : $(DRIVERS_SRC)/%.c
	$(CC) $(CFLAGS) $(INC_FLAG_DRIVERS) -c $< -o $@

# Or use -ffreestanding or -fno-hosted instead?
# They all disable -fno-tree-loop-distribute-patterns.
$(OBJDIR)/nostdlib.o : $(DRIVERS_SRC)/nostdlib.c
	$(CC) $(CFLAGS) -fno-builtin -c $< -o $@

# Demo application
$(OBJDIR)/main.o : $(APP_SRC)/main.c
	$(CC) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) -c $< -o $@

$(OBJDIR)/print.o : $(APP_SRC)/print.c
	$(CC) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) -c $< -o $@

$(OBJDIR)/receive.o : $(APP_SRC)/receive.c
	$(CC) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) -c $< -o $@


# Cleanup directives:
clean_obj :
	$(RM) -r $(OBJDIR)

clean_intermediate : clean_obj
	$(RM) $(ELF_IMAGE) $(MAPFILE) $(LISTING)

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
	QEMU_AUDIO_DRV=none qemu-system-arm -M versatilepb -nographic -m 128 -kernel $(TARGET) -d guest_errors,unimp
# -d int

qemu: run


.PHONY : all rebuild clean clean_obj clean_intermediate help run qemu
