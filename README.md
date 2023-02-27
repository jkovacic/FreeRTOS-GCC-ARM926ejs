## About
[FreeRTOS](http://www.freertos.org/) ported to [ARM Versatile Platform Baseboard](http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf),
based on the ARM926EJ-S CPU.

The current version is based on FreeRTOS 10.4.6. The port will be regularly
updated with newer versions of FreeRTOS when they are released.

The port is still at an early development stage and includes only very basic
demo tasks. More complex tasks will be included in the future.


## Prerequisites
* [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads),
based on GCC. See comments in _setenv.sh_ for more details about download and installation.
* [GNU Make](https://www.gnu.org/software/make/manual/make.html)
* [Qemu](https://www.qemu.org/)

On Debian or Ubuntu you should be ready to go with:
<pre>
sudo apt install gcc-arm-none-eabi make qemu-system-arm
</pre>

## Build
A convenience Bash script [setenv.sh](setenv.sh) is provided to set paths to toolchain's commands
and libraries. You may edit it and adjust the paths according to your setup. To set up
the necessary paths, simply type:

`. setenv.sh`

If you wish to run the image anywhere else except in Qemu, you will probably have to
edit the linker script [qemu.ld](Demo/qemu.ld) and adjust the startup address properly.

To build the image with the test application, just run _make_ or _make rebuild_.
If the build process is successful, the image file _image.bin_ will be ready to boot.

You can edit the [Makefile](Makefile) and set USE_NEWLIB=1 if you want to link against
a full libc for a bigger project using more C-functions from the standard library.

# Run
To run the target image in Qemu, enter the following command ('Ctrl-A x' to exit qemu):

`qemu-system-arm -M versatilepb -nographic -m 128 -kernel image.bin`

Or more conveniently:

`make run`

A convenience Bash script [start\_qemu.sh](start_qemu.sh) is provided. If necessary, you may
edit it and adjust paths to Qemu and/or target image.

The demo application will run infinitely so it must be stopped manually by
"killing" the instance of Qemu (an "equivalent" to switching off the board).
A convenience Bash script [stop\_qemu.sh](stop_qemu.sh) (it must be run in another shell)
is provided to automate the process. Note that it may not work properly if
multiple instances of _qemu-system-arm_ are running.
You can also just execute _killall qemu-system-arm_ in another shell to stop all qemu instances.

For more details, see extensive comments in both scripts.

## License
All source and header files are licensed under
the [MIT license](https://www.freertos.org/a00114.html).

For the avoidance of any doubt refer to the comment included at the top of each source and
header file for license and copyright information.
