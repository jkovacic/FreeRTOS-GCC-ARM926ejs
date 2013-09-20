#!/bin/bash
#
# A convenient script to terminate a running instance of "system-arm-qemu",
# an equivalent to power off a real board.
# It may be useful when the application stops and the emulated processor
# keeps running in an infinite loop.
#
# NOTE: the script might not work properly if more than one instance
# of "system-arm-qemu" is running!


# Obtain the PID of (presumably) the only running instance of "system-arm-qemu"...

# Typical line of output of "ps a" looks like this:
# 3073 pts/3    Sl+    0:04 /usr/bin/qemu-system-arm -M versatilepb -nographic -kernel image.bin
#
# The 1st element represents the process's PID, the 5th element represents its full path


PID=`ps a | awk '$5~/qemu-system-arm$/ {print $1}'`

# Once the process's PID is known, its termination is very trivial:
kill $PID
