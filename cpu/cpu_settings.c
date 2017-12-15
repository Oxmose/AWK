/*******************************************************************************
 *
 * File: cpu_settings.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 14/12/2017
 *
 * Version: 1.0
 *
 * X86 abstraction: setting functions and structures.
 * Used to set the GDT, IDT, TSS and other structures.
 ******************************************************************************/

/* Header file */
#include "cpu_settings.h"

/* memset */
#include "../lib/string.h"

/* Generic int types */
#include "../lib/stdint.h"

/* kernel_success */
#include "../core/kernel_output.h"

/* Kernel GDT structure */
extern uint64_t cpu_gdt[GDT_ENTRY_COUNT];
extern uint16_t cpu_gdt_size;
extern uint32_t cpu_gdt_base;

/**********************************
 * Kernel CPU settings functions
 *********************************/
void format_gdt_entry(uint64_t *entry, 
                      const uint32_t base, const uint32_t limit,  
                      const unsigned char type, const uint32_t flags)
{
    uint32_t lo_part = 0;
    uint32_t hi_part = 0;

    /*
     * Low part[31;0] = Base[15;0] Limit[15;0]
     */
    lo_part = ((base & 0xFFFF) << 16) | (limit & 0xFFFF);

    /* 
     * High part[7;0] = Base[23;16]
     */
    hi_part = (base >> 16) & 0xFF;
    /* 
     * High part[11;8] = Type[3;0]
     */
    hi_part |= (type & 0xF) << 8;
    /* 
     * High part[15;12] = Seg_Present[1;0]Privilege[2;0]Descriptor_Type[1;0]
     * High part[23;20] = Granularity[1;0]Op_Size[1;0]L[1;0]AVL[1;0]
     */
    hi_part |= flags & 0x00F0F000;
    
    /* 
     * High part[19;16] = Limit[19;16]
     */
    hi_part |= limit & 0xF0000;
    /* 
     * High part[31;24] = Base[31;24]
     */
    hi_part |= base & 0xFF000000;

    /* Set the value of the entry */
    *entry = lo_part | (((uint64_t) hi_part) << 32);
}

void setup_gdt(void)
{
    /* Blank the GDT, set the NULL descriptor */
    memset(cpu_gdt, 0, sizeof(uint64_t) * GDT_ENTRY_COUNT);

    /* Set the kernel code descriptor */
    uint32_t kernel_code_seg_flags = GDT_FLAG_GRANULARITY_4K | 
                                         GDT_FLAG_32_BIT_SEGMENT |
                                         GDT_FLAG_PL0 | 
                                         GDT_FLAG_SEGMENT_PRESENT |
                                         GDT_FLAG_CODE_TYPE;
    uint32_t kernel_code_seg_type = GDT_TYPE_EXECUTABLE | 
                                        GDT_TYPE_READABLE | 
                                        GDT_TYPE_PROTECTED;

    format_gdt_entry(&cpu_gdt[KERNEL_CS / 8], 
                     KERNEL_CODE_SEGMENT_BASE, KERNEL_CODE_SEGMENT_LIMIT,
                     kernel_code_seg_type, kernel_code_seg_flags);

    

    /* Set the kernel data descriptor */
    uint32_t kernel_data_seg_flags = GDT_FLAG_GRANULARITY_4K | 
                                     GDT_FLAG_32_BIT_SEGMENT |
                                     GDT_FLAG_PL0 | 
                                     GDT_FLAG_SEGMENT_PRESENT |
                                     GDT_FLAG_DATA_TYPE;

    uint32_t kernel_data_seg_type = GDT_TYPE_WRITABLE | 
                                        GDT_TYPE_GROW_DOWN;
    format_gdt_entry(&cpu_gdt[KERNEL_DS / 8], 
                     KERNEL_DATA_SEGMENT_BASE, KERNEL_DATA_SEGMENT_LIMIT,
                     kernel_data_seg_type, kernel_data_seg_flags);


    /* Set the GDT descriptor */
    cpu_gdt_size = ((sizeof(uint64_t) * GDT_ENTRY_COUNT) - 1);
    cpu_gdt_base = (uint32_t)&cpu_gdt;
    /* Load the GDT */
    __asm__ __volatile__("lgdt %0" :: "m" (cpu_gdt_size), "m" (cpu_gdt_base));

    /* Load segment selectors with a far jump for CS*/
    __asm__ __volatile__("movw %w0,%%ds" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%es" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%fs" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%gs" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%ss" :: "r" (KERNEL_DS));
    __asm__ __volatile__("ljmp %0, $flab \n\t flab: \n\t" :: "i" (KERNEL_CS));

    kernel_success("GDT Initialized\n", 16);
}