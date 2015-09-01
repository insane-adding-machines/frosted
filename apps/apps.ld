/******************************************************************************
 *
 * standalone.ld - Linker script for applications using startup.c 
 *
 *****************************************************************************/
ENTRY(_init);
MEMORY
{
    FLASH (rx) : ORIGIN = 0x8020000, LENGTH = 128K
    SRAM (rwx) : ORIGIN = 0x2000F000, LENGTH = 4K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.init*))
        *(.text*)
        *(.rodata*)
    } > FLASH
 
    .data :
    {
        apps_etext = LOADADDR(.data);
        apps_data = .;
        *(vtable)
        *(.data*)
        apps_edata = .;
    } > SRAM AT > FLASH
 
    .bss :
    {
        apps_bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(32 / 8);
        apps_ebss = .;
    } > SRAM
}
