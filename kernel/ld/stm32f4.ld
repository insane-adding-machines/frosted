ENTRY(Reset_Handler) 
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1M
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

__StackTop = ORIGIN(SRAM) + LENGTH(SRAM) - 4;
 
SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        . = ALIGN(4);
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
