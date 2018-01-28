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

/* Memory map data */
extern uint32_t    memory_map_size;
extern mem_range_t memory_map_data[];

/* Allocation tracking */
static mem_range_t current_mem_range;
static uint8_t*    next_free_frame;

/* Todo page allocator */
static uint32_t kernel_pgdir[1024] __attribute__((aligned(4096)));
static uint32_t kernel_page_tables[1024][1024] __attribute__((aligned(4096)));

static uint8_t init = 0;
static uint8_t enabled;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

__inline__ static void invalidate_tlb(void)
{
    /* Invalidate the TLB */
    __asm__ __volatile__("movl	%cr3,%eax");
	__asm__ __volatile__("movl	%eax,%cr3");
}

OS_RETURN_E init_paging(void)
{
    uint32_t i;
    uint32_t j;
    uint32_t kernel_memory_size;

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

    kernel_memory_size = KERNEL_MEMORY_KB_SIZE / KERNEL_PAGE_SIZE;
    if(KERNEL_MEMORY_KB_SIZE % KERNEL_PAGE_SIZE != 0)
    {
        ++kernel_memory_size;
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

    /* Map the first MB of the kernel, ID mapped for the kernel */
    for(i = 0; i < kernel_memory_size; ++i)
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

    /* First page is not mapped (catch NULL pointers) */
    kernel_page_tables[0][0] = PAGE_FLAG_SUPER_ACCESS |
                               PAGE_FLAG_READ_WRITE |
                               PAGE_FLAG_NOT_PRESENT;

    /* Init next free frame */
    next_free_frame = (uint8_t*)((kernel_memory_size * 1024) * 0x1000);

    /* Check bounds */
    if((uint32_t)next_free_frame < current_mem_range.base ||
       (uint32_t)next_free_frame >= current_mem_range.limit)
    {
        kernel_error("Paging Out of Bounds: request=0x%08x, base=0x%08x, limit=0x%08x\n", next_free_frame, current_mem_range.base, current_mem_range.limit);
        return OS_ERR_NO_MORE_FREE_MEM;
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

    /* Enable paging and write protect */
    __asm__ __volatile__("push %eax");
    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %cr0, %eax");
    __asm__ __volatile__("or $0x80010000, %eax");
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

    /* Disable paging and write protect */
    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %cr0, %eax");
    __asm__ __volatile__("and $0x7FF7FFFF, %eax");
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
                        const uint32_t mapping_size,
                        const uint16_t flags,
                        const uint16_t allow_remap)
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

    #ifdef DEBUG_MEM
    kernel_serial_debug("Mapping (after align) 0x%08x, to 0x%08x (%d bytes)\n",
                        virt_addr, phys_addr, mapping_size);
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
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
           PG_DIR_FLAG_PAGE_PRESENT)
        {
            page_table = (uint32_t*)kernel_page_tables[pgdir_entry];

            for(i = 0; i < 1024; ++i)
            {
                page_table[i] = PAGE_FLAG_SUPER_ACCESS |
                                PAGE_FLAG_READ_ONLY |
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

        /* Check if already mapped */
        if((*page_entry & PAGE_FLAG_PRESENT) == PAGE_FLAG_PRESENT &&
           allow_remap == 0)
        {
            #ifdef DEBUG_MEM
            kernel_serial_debug("Mapping (after align) 0x%08x, to 0x%08x (%d bytes) Already mapped)\n",
                                virt_addr, phys_addr, mapping_size);
            virt_save = (uint32_t)virt_addr;
            #endif

            return OS_ERR_MAPPING_ALREADY_EXISTS;
        }

        *page_entry = (uint32_t)phys_addr |
                      flags |
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

    invalidate_tlb();

    return OS_NO_ERR;
}

OS_RETURN_E kernel_munmap(uint8_t* virt_addr, const uint32_t mapping_size)
{
    uint32_t  pgdir_entry;
    uint32_t  pgtable_entry;
    uint32_t* page_table;
    uint32_t* page_entry;
    uint32_t  end_map;

    #ifdef DEBUG_MEM
    uint32_t virt_save;
    #endif

    /* Get end mapping addr */
    end_map = (uint32_t)virt_addr + mapping_size;

    #ifdef DEBUG_MEM
    kernel_serial_debug("Unmapping (before align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((uint32_t)virt_addr & 0xFFFFF000);

    #ifdef DEBUG_MEM
    kernel_serial_debug("Unmapping (after align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
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
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
           PG_DIR_FLAG_PAGE_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        /* Map the address */
        page_table = (uint32_t*)kernel_page_tables[pgdir_entry];
        page_entry = &page_table[pgtable_entry];

        if((*page_entry & PAGE_FLAG_PRESENT) !=
           PAGE_FLAG_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        *page_entry = PAGE_FLAG_SUPER_ACCESS |
                      PAGE_FLAG_READ_ONLY |
                      PAGE_FLAG_NOT_PRESENT;

        virt_addr += KERNEL_PAGE_SIZE;
    }

    #ifdef DEBUG_MEM
    /* Get PGDIR entry */
    pgdir_entry = (((uint32_t)virt_save) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((uint32_t)virt_save) >> 12) & 0x03FF;

    kernel_serial_debug("Unmapped 0x%08x -> 0x%08x\n", virt_save,
                        ((uint32_t*)kernel_page_tables[pgdir_entry])[pgtable_entry]);
    #endif

    invalidate_tlb();

    return OS_NO_ERR;
}
