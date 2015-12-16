 tar ext :3333
 monitor reset
 layout src
 symbol-file
 file kernel.elf
 add-symbol-file apps/apps.bflt.gdb 0x20040 -s .data 0x20009324 -s .bss 0x20009f04
 mon reset
 mon halt
 stepi
 focus c
