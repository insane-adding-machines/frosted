/******************************************************************************
 *
 * standalone.ld - Linker script for applications using startup.c 
 *
 *****************************************************************************/
ENTRY(ResetISR) 
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
 
SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH
 
    .data :
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM AT > FLASH
 
    .bss :
    {
        _bss = .;
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
        __bss_end__ = .;
        end = .;
        __end__ = .;
    } > SRAM

    .heap :
    {
        /* heap starts after BSS */
        __end__ = .;
        /* "end" is used by newlib's syscalls!!! */
        end = __end__;
        PROVIDE(_heap_start = end );
    } > SRAM
}
