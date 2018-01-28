ENTRY(loader)                /* the name of the entry label */
SECTIONS
{
    /* Kernel starts at 2M */
    . = 0x200000;
    _start = .;

    /* Put the multiboot header */
    .multiboot BLOCK(4K) : ALIGN(4K)
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
        _edata  =  .;
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

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/

    /* VESA Structures, must be placed in lower memory */
    vbe_info_base      = 0x2000; /* SIZE = 4096bits, ends at 0x2200 */
    vbe_mode_info_base = 0x2300; /* SIZE = 2048bits, ends at 0x2400 */

    /* CPU structures placed in between 1M and 2M */
    cpu_idt_size = 0x100000; /* 16 bit */
    cpu_idt_base = 0x100002; /* 32 bits*/
    cpu_idt      = 0x100006; /* SIZE = 16384bits, ends at 0x100806 */

    cpu_gdt_size = 0x100806; /* 16 bits */
    cpu_gdt_base = 0x100808; /* 32 bits */
    cpu_gdt      = 0x10080C; /* SIZE = 16384bits, ends at 0x10100C */

    /* Multiboot data pointer */
    multiboot_data_ptr = 0x10100C; /* 32bits */
    memory_map_size    = 0x101010; /* 32bits */
    memory_map_data    = 0x101014; /* SIZE = 16384bits, ends at 0x101814 */

    /* Beginning of kernel memory heap 16Mb */
    mem_heap_start = 0x1000000;

    /* End of kernel memory heap 32Mb = 16Mb */
    mem_heap_end = 0x2000000;

    /* User/Kernel SPACE Memory start */
    user_mem_start = mem_heap_end;
}
