/* Use 64 MB as the base address */
physical_load_addr = 0x4000000;
ENTRY (_start)
SECTIONS
{
    . = physical_load_addr; _physical_load_addr = physical_load_addr;
    .text :
    {
        *(.text)
        *(.rodata)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
    }

    .bss :
    {
        _sbss = .;
        *(COMMON)
        *(.bss)
        _ebss = .;
    }
}
