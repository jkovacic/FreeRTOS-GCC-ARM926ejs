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
# Only IA32 version of the toolchain is available. In order to work properly on 
# x64 systems, 'ia32-libs' must also be installed.

# Version 2014-05.28 of "Sourcery CodeBench Lite Edition for ARM EABI" 
# (now owned by Mentor Graphics) is used as the toolchain. It can be downloaded from
# http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/arm-eabi
# (a free registration is necessary to download). I downloaded the advanced package
# of IA32 GNU/Linux TAR and manually unpacked it into /opt, so all paths 
# will be relative to this one:

TOOLCHAIN=/opt/arm-2014.05

# Add a path to gnu-none-eabi-* executables:
export PATH=$PATH:$TOOLCHAIN/bin

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
