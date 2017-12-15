/*******************************************************************************
 *
 * File: cpu_settings.h
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

#ifndef __CPU_SETTINGS_H_
#define __CPU_SETTINGS_H_

/* Generic int types */
#include "../lib/stdint.h"

/**********************************
 * Kernel CPU constants
 *********************************/

/* GDT Settings */
#define GDT_ENTRY_COUNT 3

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10

#define KERNEL_CODE_SEGMENT_BASE  0x00000000
#define KERNEL_CODE_SEGMENT_LIMIT 0x000FFFFF
#define KERNEL_DATA_SEGMENT_BASE  0x00000000
#define KERNEL_DATA_SEGMENT_LIMIT 0x000FFFFF

/* GDT Flags */
#define GDT_FLAG_GRANULARITY_4K   0x800000
#define GDT_FLAG_GRANULARITY_BYTE 0x000000
#define GDT_FLAG_16_BIT_SEGMENT   0x000000
#define GDT_FLAG_32_BIT_SEGMENT   0x400000
#define GDT_FLAG_64_BIT_SEGMENT   0x200000
#define GDT_FLAG_AVL              0x100000
#define GDT_FLAG_SEGMENT_PRESENT  0x008000
#define GDT_FLAG_PL0              0x000000
#define GDT_FLAG_PL1              0x002000
#define GDT_FLAG_PL2              0x004000
#define GDT_FLAG_PL3              0x006000
#define GDT_FLAG_CODE_TYPE        0x001000
#define GDT_FLAG_DATA_TYPE        0x001000
#define GDT_FLAG_SYSTEM_TYPE      0x000000

#define GDT_TYPE_EXECUTABLE       0x8
#define GDT_TYPE_GROW_UP          0x4
#define GDT_TYPE_GROW_DOWN        0x0
#define GDT_TYPE_CONFORMING       0x4
#define GDT_TYPE_PROTECTED        0x0
#define GDT_TYPE_READABLE         0x2
#define GDT_TYPE_WRITABLE         0x2
#define GDT_TYPE_ACCESSED         0x1

/**********************************
 * Kernel CPU structures 
 *********************************/

/**********************************
 * Kernel CPU settings functions
 *********************************/
/* Format data given as parameter into a standard GDT entry. The result is 
 * directly written in the memory pointed by the entry parameter.
 * @param entry The pointer to the entry structure to format.
 * @param base  The base address of the segment for the GDT entry.
 * @param limit The limit address of the segment for the GDT entry.
 * @param type  The type of segment for the GDT entry.
 * @param flags The flags to be set for the GDT entry.
 */
void format_gdt_entry(uint64_t *entry, 
                      const uint32_t base, const uint32_t limit,  
                      const uint8_t type, const uint32_t flags);

/* Setup a flat GDT for the kernel. Fills the entries in the GDT table and load
 * the new GDT. Set the segment registers (CS, DS, ES, FS, GS, SS).
 */
void setup_gdt(void);

#endif /* __CPU_SETTINGS_H_ */