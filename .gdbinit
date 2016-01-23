 tar ext :3333
 monitor reset
 layout src
 symbol-file
 file kernel.elf
 add-symbol-file ./frosted-mini-userspace-bflt/init.gdb 0x10090 -s .data 0x20008014 -s .bss 0x20008954
 mon reset
 mon halt
 stepi
 focus c
