## About
[FreeRTOS](http://www.freertos.org/) ported to [ARM Versatile Platform Baseboard](http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf),
based on the ARM926EJ-S CPU.

The current version is based on FreeRTOS 10.2.1. The port will be regularly
updated with newer versions of FreeRTOS when they are released.

The port is still at an early development stage and includes only very basic
demo tasks. More complex tasks will be included in the future.


## Prerequisites
* _[GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)_,
based on GCC. See comments in _setenv.sh_ for more details about download and installation.
* _GNU Make_
* _Qemu_ (version 1.3 or newer, older versions do not emulate the interrupt controller properly!)

## Build
A convenience Bash script _setenv.sh_ is provided to set paths to toolchain's commands
and libraries. You may edit it and adjust the paths according to your setup. To set up
the necessary paths, simply type:

`. ./setenv.sh`

If you wish to run the image anywhere else except in Qemu, you will probably have to
edit the linker script _qemu.ld_ and adjust the startup address properly.

To build the image with the test application, just run _make_ or _make rebuild_.
If the build process is successful, the image file _image.bin_ will be ready to boot.

# Run
To run the target image in Qemu, enter the following command:

`qemu-system-arm -M versatilepb -nographic -m 128 -kernel image.bin`

A convenience Bash script _start\_qemu.sh_ is provided. If necessary, you may
edit it and adjust paths to Qemu and/or target image.

The demo application will run infinitely so it must be stopped manually by
"killing" the instance of Qemu (an "equivalent" to switching off the board).
A convenience Bash script _stop\_qemu.sh_ (it must be run in another shell)
is provided to automate the process. Note that it may not work properly if
multiple instances of _qemu-system-arm_ are running.

For more details, see extensive comments in both scripts.

## License
All source and header files are licensed under
the [MIT license](https://www.freertos.org/a00114.html).

For the avoidance of any doubt refer to the comment included at the top of each source and
header file for license and copyright information.
