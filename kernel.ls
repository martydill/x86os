/* kernel.ls */

physical_load_addr = 0x00101000;
ENTRY (_start)

SECTIONS
{
    . = physical_load_addr;
	.multibootHeader : {
      *(.multibootHeader)
   }
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

    kernelEndAddress = .; _kernelEndAddress = .; __kernelEndAddress = .;
}
