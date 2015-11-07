ENTRY(Reset_Handler) 
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 512K
    SRAM (rwx) : ORIGIN = 0x100000C8, LENGTH = (32K - 0xC8)
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
        *(.bss*)
        *(COMMON)
        . = ALIGN(32 / 8);
        _ebss = .;
        PROVIDE (end = .);
    } > SRAM

    .heap :
    {
        /* heap starts after BSS */
        PROVIDE(_heap_start = _ebss );
    } > SRAM
}
