/******************************************************************************
 *
 * standalone.ld - Linker script for applications using startup.c 
 *
 *****************************************************************************/
ENTRY(Reset_Handler) 
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

__StackTop = ORIGIN(SRAM) + LENGTH(SRAM) - 4;
 
SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        KEEP(*(.syscall_vector))
        *(.text*)
        *(.rodata*)
    } > FLASH
 
    .data :
    {
        _etext = LOADADDR(.data);
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
        . = ALIGN(32 / 8);
        _ebss = .;
        _end = .;
        _bss_end__ = . ; __bss_end__ = . ; __end__ = . ;
        PROVIDE (end = .);
    } > SRAM

    .heap :
    {
        /* heap starts after BSS */
        PROVIDE(_heap_start = _end );
    } > SRAM
}
