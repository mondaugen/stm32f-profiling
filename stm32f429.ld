/* ENTRY(Reset_Handler); */

MEMORY
{
    /* Programs reside in bank 1, leaving bank 2 free to record into. */
    flash (rx) : ORIGIN = 0x08000000, LENGTH = 1M
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 192K
}

_ram_start = ORIGIN(ram);
/* _estack    = ORIGIN(ram) + LENGTH(ram) - 4; */
_estack = 0x2002ffff;
_Min_Heap_Size = 32K;
_Min_Stack_Size = 32K;

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        *(.isr_vector)
        . = ALIGN(4);
    } >flash
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        . = ALIGN(4);
        _etext = .;
    } >flash
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >ram AT> flash
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        /* Needed by standard library, it seems */
        __bss_start__ = .;
        *(.bss)
        *(.bss*)
        . = ALIGN(4);
        _ebss = .;
        /* Needed by standard library, it seems */
        __bss_end__ = .;
    } >ram
    ._user_heap_stack :
    {
        . = ALIGN(4);
        _end = .;
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(4);
    } >ram
    /DISCARD/ :
    {
        libc.a (*)
        libm.a (*)
        libgcc.a (*)
    }
}
