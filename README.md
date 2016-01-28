# Frosted 

### Introduction

frosted is an acronym that means "Free Operating Systems for Tiny Embedded Devices"

The goal of this project is to provide a Free (as in Freedom) kernel for embedded systems, 
which exposes a POSIX-compliant system call API.

From an architectural point of view, we have chosen to introduce a strong separation between
user space and kernel space code. To do so, the kernel is compiled as a standalone ELF file. 

The project borrows Linux Kernel's kconfig to allow for a graphic configuration and selection 
of the components.

### License

All the kernel code is released under the terms of [GNU GPLv2](http://www.gnu.org/licenses/gpl-2.0.html).

Linking on top of the system call interface, however, does not constitute a derivative work, as calling
the system calls interface exposed by the kernel constitutes regular kernel usage, as long as the resulting
work is not linked in the same binary as the kernel. This means that you are allowed to use the frosted
kernel without any restriction about distributing your user space application source code.


### Hardware support and toolchains

Frosted can be compiled using the [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) toolchain.

At the moment, only ARM Cortex M0/M3/M4/M7 CPUs are supported.

The kernel relies on [libopencm3](https://github.com/libopencm3/libopencm3/) to provide hardware abstraction. 
The frosted team maintains their own [libopencm3 branch](https://github.com/insane-adding-machines/libopencm3/)
to guarantee the compatibility with the kernel. However, we try to contribute to the original libopencm3 project
as we attempt to propose our changes for acceptance in the master repository.

Here is a list of the platforms that are currently supported:

 * LM3S (e.g. qemu-system-arm or TI stellaris)
 * STM32F4 (e.g. ST Discovery boards)
 * LPC17XX (e.g. Seeed studio ArchPRO or early 1768-Mbed boards)

### Testing on the synthetic target

The requirements to test frosted on synthetic target are:
 * A Linux distribution, or a POSIX-compliant OS which supports the tools described below
 * the [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) toolchain (version 4.9.x or greater)
 * a QEMU installation supporting qemu-system-arm (version 2.2.x or later)
 * The GCC ARM Embedded debugger (gdb), for kernel debugging

How to run the QEMU example synthetic target:
 * Use ```make menuconfig``` to select preferred options. For running under qemu, the CPU type has to be set to "LM3S"
 * Use ```make``` to start the build
 * Use ```make qemu2``` to run frosted in a virtual machine.
 * Use ```make qemu``` to run frosted in a virtual machine. The kernel will wait until a GDB debugger is connected to local port 3333.


### Compiling userland

The frosted-mini-userspace can be built with the GCC ARM Embedded toolchain, and it will produce a single, standalone elf 
binary that can be flashed separately from the kernel.

Unless you are compiling applications in a single elf, you need the arm-frosted-gcc toolchain 
to generate userland binaries and libraries (e.g. to compile the frosted-mini-userspace-bflt).

[Get the latest frosted userland toolchain here](https://github.com/insane-adding-machines/crosstool-ng/releases/tag/v16.01).


### Joining the team / contacts

We are constantly looking for new developers to join the team. Please contact us if you are interested.

The development team meets regularly on IRC (irc://irc.freenode.net/#frosted).
As an alternative, please use the [issue board](https://github.com/insane-adding-machines/frosted/issues) on github to contact us.

Enjoy!


_the frosted development team_
