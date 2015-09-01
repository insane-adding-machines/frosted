 tar ext :3333
 layout src
 file kernel.elf
 add-symbol-file apps.elf 0x20000
 stepi
 focus c
