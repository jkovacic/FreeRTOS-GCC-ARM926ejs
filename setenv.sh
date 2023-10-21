#!/bin/bash
#
# A convenience script that sets environment variables, necessary
# to use the GNU toolchain for the ARM architecture.
#
# This script sets PATH and C_INCLUDE_PATH. If you intend to link
# the toolchain's libraries (e.g. libgcc.a), other variables
# (e.g. LIBRARY_PATH) must be set as well.
#
# Make sure, you set the environment variables appropriately for your
# setup. Typically, setting TOOLCHAIN only should be enough.
#
# IMPORTANT: this script must be run as 
#     . ./setenv.sh
# or its longer equivalent:
#     source ./setenv.sh
#
# otherwise the variables will be discarded immediately after the script completes!


# NOTE:
# For Linux, only 64-bit version of the toolchain is available. If building
# on a 32-bit Linux host, consider using an older version of the toolchain or
# building it from the provided source code.

# Version "6-2017-q2-update" of "GNU Arm Embedded Toolchain" 
# is used as the toolchain. It can be downloaded from
# https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
# I downloaded the Linux 64-bit build of the toolchain and manually unpacked
# it into /opt, so all paths will be relative to this one:

#TOOLCHAIN=/opt/gcc-arm-none-eabi-9-2020-q2-update
TOOLCHAIN=/opt/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-eabi

# Add a path to gnu-none-eabi-* executables:
export PATH=$TOOLCHAIN/bin:$PATH

# After the script completes, you may check that output of
#    echo $PATH
# includes the desired path. Additionally you may check if arm-none-eabi-gcc
# is found if you attempt to run this executable.


# If you have gcc installed, C_INCLUDE_PATH might be set to its include paths.
# This may be confusing when you build ARM applications, therefore this variable
# (if it exists) will be overwritten:
export C_INCLUDE_PATH=$TOOLCHAIN/arm-none-eabi/include

# After the script completes, check the effect of this variable by executing:
# `arm-none-eabi-gcc -print-prog-name=cc1` -v
#
# More info about this at:
# http://stackoverflow.com/questions/344317/where-does-gcc-look-for-c-and-c-header-files

# Export other environment variables (e.g. LIBRARY_PATH) if necessary.


# Variable TOOLCHAIN not needed anymore, it can be unset
unset TOOLCHAIN
