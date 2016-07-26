# Frosted
 * Kernel build status:       [![Build   Status](http://trexotech.com:8080/buildStatus/icon?job=frosted-multi&style=plastic)] (http://trexotech.com:8080/job/frosted-multi/)

 * Toolchain build status:    [![Toolchain Status](http://trexotech.com:8080/buildStatus/icon?job=arm-frosted-eabi-from-source&style=plastic)] (http://trexotech.com:8080/job/arm-frosted-eabi-from-source/?style=plastic)

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

### Hardware Support

[Hardware support](https://github.com/insane-adding-machines/frosted/wiki/Hardware-support)

### Getting started

[Getting started](https://github.com/insane-adding-machines/frosted/wiki/Getting-started)

### Joining the team / contacts

We are constantly looking for new developers to join the team. Please contact us if you are interested.

The development team meets regularly on IRC (irc://irc.freenode.net/#frosted).
As an alternative, please use the [issue board](https://github.com/insane-adding-machines/frosted/issues) on github to contact us.

Enjoy!


_the frosted development team_
