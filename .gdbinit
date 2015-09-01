 tar ext :1234
 monitor reset
 layout src
 file kernel.elf
 add-symbol-file apps.elf 0x20000
 mon reset
 mon halt
 stepi
 focus c
