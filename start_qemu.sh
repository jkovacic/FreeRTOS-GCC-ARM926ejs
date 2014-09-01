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
# Qemu version 1.3 or newer is required as previous versions of Qemu do not
# emulate the the PL190 (vector interrupt controller) properly. Most
# Linux distributions should include quite a recent version of Qemu.
# If this is not the case (e.g. if you use Debian), you should build
# a more recent version of Qemu yourself (it is sufficient to build only
# the ARM emulator) and modify the QEMUBIN variable below to the full path
# to your binary "qemu-system-arm". The commneted example below assumes you
# have "installed" it to your home directory under ~/qemu.

QEMUBIN=qemu-system-arm
#QEMUBIN=~/qemu/bin/qemu-system-arm

$QEMUBIN -M versatilepb -nographic -m 128 -kernel $IMAGE_FILE $QEMU_GDB_ARG
