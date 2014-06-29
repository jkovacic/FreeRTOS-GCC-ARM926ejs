#!/bin/bash
#
# Usage: start_qemu.sh [image_file] [-debug]
#
# Runs an instance of "qemu-system-arm", emulating an ARM Versatile Application Baseboard with
# ARM926EJ-S. Qemu is run in the "nographics mode", i.e. its display will not open and the
# board's UART0 serial port will be "connected" directly to the running shell's standard output
# and/or input.
#
# If no image_file argument is provided, "image.bin" (as built by Makefile) is executed as
# a firmware image file.
#
# If '-debug' is specified, the Qemu will be executed with additional "-S -s" flags.
# This will cause Qemu to act as a GDB server, listening on the TCP port 1234.
# Execution of the image will freeze at the startup point.
#
# For more details, see:
# http://winfred-lu.blogspot.com/2011/12/arm-u-boot-on-qemu.html

# Default firmware image if no other one is provided:
DEF_IMAGE_FILE=image.bin

# Qemu arguments to invoke Qemu with a GDB server listening on the 
# TCP port 1234 and freeze execution of the image at startup.
QEMU_GDB='-S -s'

# Initial values of both Qemu arguments:
QEMU_GDB_ARG=''
IMAGE_FILE=''

# Check all CLI arguments to this script. The first argument, not equal
# to "-debug", will be considered as the desired image name. All 
# remaining arguments, not equal to "-debug", will be discarded.
 
for arg in $*; do
  if [ "$arg" = '-debug' ]; then
    QEMU_GDB_ARG=$QEMU_GDB;
  else
    if [ "$IMAGE_FILE" = '' ]; then
      IMAGE_FILE=$arg
    fi
  fi
done

# If no image file has been provided, assign it the default name.
if [ "$IMAGE_FILE" = '' ]; then
  IMAGE_FILE=$DEF_IMAGE_FILE
fi

# NOTE:
# The version of Qemu (1.0), delivered with my distribution of Linux, does not emulate the
# PL190 (vector interrupt controller) properly. For that reason I have built a more recent
# version of Qemu (1.6) and "installed" it into my home directory under ~/qemu.
# This directory is not in my PATH, so I provide the full path in QEMUBIN.
#
# If this bug is already fixed in your version of Qemu, you may simply uncomment the 
# first definition of QEMUBIN and comment out the second one. Otherwise just adjust the
# second definition according to your setup.

#QEMUBIN=qemu-system-arm
QEMUBIN=~/qemu/bin/qemu-system-arm

$QEMUBIN -M versatilepb -nographic -m 128 -kernel $IMAGE_FILE $QEMU_GDB_ARG
