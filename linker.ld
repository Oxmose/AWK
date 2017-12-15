ENTRY(loader)                /* the name of the entry label */
SECTIONS
{
    /* Kernel starts at 1M */
    . = 0x100000;

    _start = .;

    /* KERNEL MUST BE ALIGNED ON 4K */

    /* Put the multiboot header */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
    }
 
    /* Executable code */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }
 
    /* Read-only data. */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }
 
    /* Read-write data */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }
 
    /* Read-write data and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }

    _end = .;

     /* CPU structure placed in lower memory */
    cpu_idt = 0x01000; /* SIZE = 16384bits */

    cpu_gdt_size = 0x10000; /* 16 bits */
    cpu_gdt_base = 0x10002; /* 32 bits */    
    cpu_gdt      = 0x10006; /* SIZE = SHOULD BE LESS THAN 65488Bits */


    cpu_tss = 0x20000; /* Kernel primary TSS */
}