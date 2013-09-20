#!/bin/bash
#
# Usage: start_qemu.sh [image_file]
#
# Runs an instance of "qemu-system-arm", emulating an ARM Versatile Application Baseboard with
# ARM926EJ-S. Qemu is run in the "nographics mode", i.e. its display will not open and the
# board's UART0 serial port will be "connected" directly to the running shell's standard output
# and/or input.
#
# If no command line argument is provided, "image.bin" (as built by Makefile) is executed as
# a firmware image file.

# Default firmware image:
IMAGE_FILE=image.bin

# Assign the first argument to IMAGE_FILE if at least one CLI argument is provided:
if [ $# -ge 1 ]; then
    IMAGE_FILE=$1;
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

$QEMUBIN -M versatilepb -nographic -m 128 -kernel $IMAGE_FILE
