/*******************************************************************************
 *
 * File: paging.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/01/2018
 *
 * Version: 1.0
 *
 * Kernel memory paging manager.
 ******************************************************************************/

#ifndef __PAGING_H_
#define __PAGING_H_

#include "../lib/stddef.h" /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define KERNEL_PAGE_SIZE      4096
#define KERNEL_MEMORY_KB_SIZE 32768

#define PG_DIR_FLAG_PAGE_SIZE_4KB       0x00000000
#define PG_DIR_FLAG_PAGE_SIZE_4MB       0x00000080
#define PG_DIR_FLAG_PAGE_ACCESSED       0x00000020
#define PG_DIR_FLAG_PAGE_CACHE_DISABLED 0x00000010
#define PG_DIR_FLAG_PAGE_CACHE_WT       0x00000008
#define PG_DIR_FLAG_PAGE_CACHE_WB       0x00000000
#define PG_DIR_FLAG_PAGE_USER_ACCESS    0x00000004
#define PG_DIR_FLAG_PAGE_SUPER_ACCESS   0x00000000
#define PG_DIR_FLAG_PAGE_READ_WRITE     0x00000002
#define PG_DIR_FLAG_PAGE_READ_ONLY      0x00000000
#define PG_DIR_FLAG_PAGE_PRESENT        0x00000001
#define PG_DIR_FLAG_PAGE_NOT_PRESENT    0x00000000

#define PAGE_FLAG_GLOBAL                0x00000100
#define PAGE_FLAG_DIRTY                 0x00000080
#define PAGE_FLAG_ACCESSED              0x00000020
#define PAGE_FLAG_CACHE_DISABLED        0x00000010
#define PAGE_FLAG_CACHE_WT              0x00000008
#define PAGE_FLAG_CACHE_WB              0x00000000
#define PAGE_FLAG_USER_ACCESS           0x00000004
#define PAGE_FLAG_SUPER_ACCESS          0x00000000
#define PAGE_FLAG_READ_WRITE            0x00000002
#define PAGE_FLAG_READ_ONLY             0x00000000
#define PAGE_FLAG_PRESENT               0x00000001
#define PAGE_FLAG_NOT_PRESENT           0x00000000

/*******************************************************************************
* STRUCTURES
******************************************************************************/

typedef struct mem_range
{
    uint32_t base;
    uint32_t limit;
    uint32_t type;
} mem_range_t;

/*******************************************************************************
* FUNCTIONS
******************************************************************************/

OS_RETURN_E init_paging(void);

OS_RETURN_E enable_paging(void);

OS_RETURN_E disable_paging(void);

OS_RETURN_E kernel_mmap(uint8_t* virt_addr, uint8_t* phys_addr,
                        const uint32_t mapping_size,
                        const uint16_t flags,
                        const uint16_t allow_remap);

OS_RETURN_E kernel_munmap(uint8_t* virt_addr, const uint32_t mapping_size);

#endif /* __PAGING_H_ */
