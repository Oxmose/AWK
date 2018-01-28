/*******************************************************************************
 *
 * File: paging.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/01/2018
 *
 * Version: 1.0
 *
 * Kernel memory paging manager.
 ******************************************************************************/

#include "../lib/stdint.h"     /* Generic int types */
#include "../lib/stddef.h"     /* OS_RETURN_E */
#include "../boot/multiboot.h" /* MULTIBOOT_MEMORY_AVAILABLE */
#include "heap.h"              /* kmalloc kfree */

#include "../debug.h"            /* DEBUG */

/* Header file */
#include "paging.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

extern uint32_t    memory_map_size;
extern mem_range_t memory_map_data[];

static mem_range_t current_mem_range;

/* Todo page allocator */
static uint32_t kernel_pgdir[1024] __attribute__((aligned(4096)));
static uint32_t kernel_page_tables[1024][1024] __attribute__((aligned(4096)));

static uint8_t init = 0;
static uint8_t enabled;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E init_paging(void)
{
    uint32_t i;
    uint32_t j;

    /* Get the first range that is free */
    for(i = 0; i < memory_map_size; ++i)
    {
        if(memory_map_data[i].base != 0x00000000 &&
           memory_map_data[i].type == MULTIBOOT_MEMORY_AVAILABLE)
           {
               current_mem_range.base  = memory_map_data[i].base;
               current_mem_range.limit = memory_map_data[i].limit;
               current_mem_range.type  = memory_map_data[i].type;
               break;
           }
    }

    if(i == memory_map_size)
    {
        return OS_ERR_NO_MORE_FREE_MEM;
    }

    #ifdef DEBUG_MEM
    kernel_serial_debug("Selected free memory range: \n");
    kernel_serial_debug("\tBase 0x%08x, Limit 0x%08x, Length %uKB, Type %d\n",
                current_mem_range.base,
                current_mem_range.limit,
                (current_mem_range.limit - current_mem_range.base) / 1024,
                current_mem_range.type);
    #endif

    /* Init kernel pgdir */
    for(i = 0; i < 1024; ++i)
    {
        kernel_pgdir[i] = PG_DIR_FLAG_PAGE_SIZE_4KB |
                          PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                          PG_DIR_FLAG_PAGE_READ_WRITE |
                          PG_DIR_FLAG_PAGE_NOT_PRESENT;
    }

    /* Map the first 32MB of the kernel, ID mapped for the kernel */
    for(i = 0; i < 8; ++i)
    {
        for(j = 0; j < 1024; ++j)
        {
            kernel_page_tables[i][j] = (((i * 1024) + j) * 0x1000) |
                                       PAGE_FLAG_SUPER_ACCESS |
                                       PAGE_FLAG_READ_WRITE |
                                       PAGE_FLAG_PRESENT;
        }
        kernel_pgdir[i] = (uint32_t)(kernel_page_tables[i]) |
                          PG_DIR_FLAG_PAGE_SIZE_4KB |
                          PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                          PG_DIR_FLAG_PAGE_READ_WRITE |
                          PG_DIR_FLAG_PAGE_PRESENT;
    }

    /* Set CR3 register */
    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %%eax, %%cr3": :"a"(kernel_pgdir));
    __asm__ __volatile__("mov %ebp, %esp");
    __asm__ __volatile__("pop %ebp");

    #ifdef DEBUG_MEM
    kernel_serial_debug("CR3 Set to 0x%08x \n", kernel_pgdir);
    #endif

    enabled = 0;
    init = 1;

    return enable_paging();
}

OS_RETURN_E enable_paging(void)
{
    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    if(enabled == 1)
    {
        return OS_NO_ERR;
    }

    __asm__ __volatile__("push %eax");
    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %cr0, %eax");
    __asm__ __volatile__("or $0x80000000, %eax");
    __asm__ __volatile__("mov %eax, %cr0");
    __asm__ __volatile__("mov %ebp, %esp");
    __asm__ __volatile__("pop %ebp");
    __asm__ __volatile__("pop %eax");

    #ifdef DEBUG_MEM
    kernel_serial_debug("Paging enabled\n");
    #endif

    enabled = 1;

    return OS_NO_ERR;
}

OS_RETURN_E disable_paging(void)
{
    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    if(enabled == 0)
    {
        return OS_NO_ERR;
    }

    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %cr0, %eax");
    __asm__ __volatile__("and $0x7FFFFFFF, %eax");
    __asm__ __volatile__("mov %eax, %cr0");
    __asm__ __volatile__("mov %ebp, %esp");
    __asm__ __volatile__("pop %ebp");

    #ifdef DEBUG_MEM
    kernel_serial_debug("Paging disabled\n");
    #endif

    enabled = 0;

    return OS_NO_ERR;
}

OS_RETURN_E kernel_mmap(uint8_t* virt_addr, uint8_t* phys_addr,
                        const uint32_t mapping_size)
{
    uint32_t  pgdir_entry;
    uint32_t  pgtable_entry;
    uint32_t* page_table;
    uint32_t* page_entry;
    uint32_t  end_map;
    uint32_t  i;

    #ifdef DEBUG_MEM
    uint32_t virt_save;
    #endif

    /* Get end mapping addr */
    end_map = (uint32_t)virt_addr + mapping_size;

    #ifdef DEBUG_MEM
    kernel_serial_debug("Mapping (before align) 0x%08x, to 0x%08x (%d bytes)\n",
                        virt_addr, phys_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((uint32_t)virt_addr & 0xFFFFF000);
    phys_addr = (uint8_t*)((uint32_t)phys_addr & 0xFFFFF000);

    kernel_serial_debug("Mapping (after align) 0x%08x, to 0x%08x (%d bytes)\n",
                        virt_addr, phys_addr, mapping_size);

    #ifdef DEBUG_MEM
    virt_save = (uint32_t)virt_addr;
    #endif

    /* Map all pages needed */
    while((uint32_t)virt_addr < end_map)
    {
        /* Get PGDIR entry */
        pgdir_entry = (((uint32_t)virt_addr) >> 22);
        /* Get PGTABLE entry */
        pgtable_entry = (((uint32_t)virt_addr) >> 12) & 0x03FF;

        /* If page table not present createe it */
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != PG_DIR_FLAG_PAGE_PRESENT)
        {
            page_table = (uint32_t*)kernel_page_tables[pgdir_entry];

            for(i = 0; i < 1024; ++i)
            {
                page_table[i] = PAGE_FLAG_SUPER_ACCESS |
                                PAGE_FLAG_READ_WRITE |
                                PAGE_FLAG_NOT_PRESENT;
            }

            kernel_pgdir[pgdir_entry] = (uint32_t)page_table |
                                        PG_DIR_FLAG_PAGE_SIZE_4KB |
                                        PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                                        PG_DIR_FLAG_PAGE_READ_WRITE |
                                        PG_DIR_FLAG_PAGE_PRESENT;
        }

        /* Map the address */
        page_table = (uint32_t*)kernel_page_tables[pgdir_entry];
        page_entry = &page_table[pgtable_entry];
        *page_entry = (uint32_t)phys_addr |
                          PAGE_FLAG_SUPER_ACCESS |
                          PAGE_FLAG_READ_WRITE |
                          PAGE_FLAG_PRESENT;

        virt_addr += KERNEL_PAGE_SIZE;
        phys_addr += KERNEL_PAGE_SIZE;
    }

    #ifdef DEBUG_MEM
    /* Get PGDIR entry */
    pgdir_entry = (((uint32_t)virt_save) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((uint32_t)virt_save) >> 12) & 0x03FF;

    kernel_serial_debug("Mapped 0x%08x -> 0x%08x\n", virt_save,
                        ((uint32_t*)kernel_page_tables[pgdir_entry])[pgtable_entry]);
    #endif

    /* Flush the TLB */
    __asm__ __volatile__("movl	%cr3,%eax");
	__asm__ __volatile__("movl	%eax,%cr3");

    return OS_NO_ERR;
}
