# Copyright 2013, Jernej Kovacic
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


#
# Type "make help" for more details.
#


# Version 2014-05.28 of the Sourcery toolchain is used as a build tool.
# See comments in "setenv.sh" for more details about downloading it
# and setting the appropriate environment variables.

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
WFLAG = -Wall -Wextra -Werror
CFLAGS = $(CPUFLAG) $(WFLAG)

# Additional C compiler flags to produce debugging symbols
DEB_FLAG = -g -DDEBUG


# Compiler/target path in FreeRTOS/Source/portable
PORT_COMP_TARG = GCC/ARM926EJ-S/

# Intermediate directory for all *.o and other files:
OBJDIR = obj/

# FreeRTOS source base directory
FREERTOS_SRC = FreeRTOS/Source/

# Directory with memory management source files
FREERTOS_MEMMANG_SRC = $(FREERTOS_SRC)portable/MemMang/

# Directory with platform specific source files
FREERTOS_PORT_SRC = $(FREERTOS_SRC)portable/$(PORT_COMP_TARG)

# Directory with HW drivers' source files
DRIVERS_SRC = drivers/

# Directory with demo specific source (and header) files
APP_SRC = Demo/


# Object files to be linked into an application
# Due to a large number, the .o files are arranged into logical groups:

FREERTOS_OBJS = queue.o list.o tasks.o
# The following o. files are only necessary if
# certain options are enabled in FreeRTOSConfig.h
#FREERTOS_OBJS += timers.o
#FREERTOS_OBJS += croutine.o
#FREERTOS_OBJS += event_groups.o

# Only one memory management .o file must be uncommented!
FREERTOS_MEMMANG_OBJS = heap_1.o
#FREERTOS_MEMMANG_OBJS = heap_2.o
#FREERTOS_MEMMANG_OBJS = heap_3.o
#FREERTOS_MEMMANG_OBJS = heap_4.o
#FREERTOS_MEMMANG_OBJS = heap_5.o

FREERTOS_PORT_OBJS = port.o portISR.o
STARTUP_OBJ = startup.o
DRIVERS_OBJS = timer.o interrupt.o uart.o

APP_OBJS = init.o main.o print.o receive.o
# nostdlib.o must be commented out if standard lib is going to be linked!
APP_OBJS += nostdlib.o


# All object files specified above are prefixed the intermediate directory
OBJS = $(addprefix $(OBJDIR), $(STARTUP_OBJ) $(FREERTOS_OBJS) $(FREERTOS_MEMMANG_OBJS) $(FREERTOS_PORT_OBJS) $(DRIVERS_OBJS) $(APP_OBJS))

# Definition of the linker script and final targets
LINKER_SCRIPT = $(addprefix $(APP_SRC), qemu.ld)
ELF_IMAGE = image.elf
TARGET = image.bin

# Include paths to be passed to $(CC) where necessary
INC_FREERTOS = $(FREERTOS_SRC)include/
INC_DRIVERS = $(DRIVERS_SRC)include/

# Complete include flags to be passed to $(CC) where necessary
INC_FLAGS = $(INCLUDEFLAG)$(INC_FREERTOS) $(INCLUDEFLAG)$(APP_SRC) $(INCLUDEFLAG)$(FREERTOS_PORT_SRC)
INC_FLAG_DRIVERS = $(INCLUDEFLAG)$(INC_DRIVERS)

# Dependency on HW specific settings
DEP_BSP = $(INC_DRIVERS)bsp.h


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
	$(LD) -nostdlib -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@

debug : _debug_flags all

debug_rebuild : _debug_flags rebuild

_debug_flags :
	$(eval CFLAGS += $(DEB_FLAG))


# Startup code, implemented in assembler

$(OBJDIR)startup.o : $(APP_SRC)startup.s
	$(AS) $(CPUFLAG) $< $(OFLAG) $@


# FreeRTOS core

$(OBJDIR)queue.o : $(FREERTOS_SRC)queue.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)list.o : $(FREERTOS_SRC)list.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)tasks.o : $(FREERTOS_SRC)tasks.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)timers.o : $(FREERTOS_SRC)timers.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)croutine.o : $(FREERTOS_SRC)croutine.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)event_groups.o : $(FREERTOS_SRC)event_groups.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@


# HW specific part, in FreeRTOS/Source/portable/$(PORT_COMP_TARGET)

$(OBJDIR)port.o : $(FREERTOS_PORT_SRC)port.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)portISR.o : $(FREERTOS_PORT_SRC)portISR.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@


# Rules for all MemMang implementations are provided
# Only one of these object files must be linked to the final target

$(OBJDIR)heap_1.o : $(FREERTOS_MEMMANG_SRC)heap_1.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_2.o : $(FREERTOS_MEMMANG_SRC)heap_2.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_3.o : $(FREERTOS_MEMMANG_SRC)heap_3.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_4.o : $(FREERTOS_MEMMANG_SRC)heap_4.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_5.o : $(FREERTOS_MEMMANG_SRC)heap_5.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@


# Drivers

$(OBJDIR)timer.o : $(DRIVERS_SRC)timer.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)interrupt.o : $(DRIVERS_SRC)interrupt.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)uart.o : $(DRIVERS_SRC)uart.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

# Demo application

$(OBJDIR)main.o : $(APP_SRC)main.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)init.o : $(APP_SRC)init.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)print.o : $(APP_SRC)print.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)receive.o : $(APP_SRC)receive.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)nostdlib.o : $(APP_SRC)nostdlib.c
	$(CC) $(CFLAG) $(CFLAGS) $< $(OFLAG) $@


# Cleanup directives:

clean_obj :
	$(RM) -r $(OBJDIR)

clean_intermediate : clean_obj
	$(RM) *.elf
	$(RM) *.img

clean : clean_intermediate
	$(RM) *.bin

# Short help instructions:

help :
	@echo
	@echo Valid targets:
	@echo - all: builds missing dependencies and creates the target image \'$(TARGET)\'.
	@echo - rebuild: rebuilds all dependencies and creates the target image \'$(TARGET)\'.
	@echo - debug: same as \'all\', also includes debugging symbols to \'$(ELF_IMAGE)\'.
	@echo - debug_rebuild: same as \'rebuild\', also includes debugging symbols to \'$(ELF_IMAGE)\'.
	@echo - clean_obj: deletes all object files, only keeps \'$(ELF_IMAGE)\' and \'$(TARGET)\'.
	@echo - clean_intermediate: deletes all intermediate binaries, only keeps the target image \'$(TARGET)\'.
	@echo - clean: deletes all intermediate binaries, incl. the target image \'$(TARGET)\'.
	@echo - help: displays these help instructions.
	@echo

.PHONY : all rebuild clean clean_obj clean_intermediate debug debug_rebuild _debug_flags help
