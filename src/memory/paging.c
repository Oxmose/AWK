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

#include "../debug.h"            /* DEBUG */

/* Header file */
#include "paging.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

 extern uint32_t    memory_map_size;
 extern mem_range_t memory_map_data[];

 static mem_range_t current_mem_range;

 static uint8_t init = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E init_paging(void)
{
    uint32_t i;

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

    #ifdef DEBUG_MEM
    kernel_serial_debug("Selected free memory range: \n");
    kernel_serial_debug("\tBase 0x%08x, Limit 0x%08x, Length %uKB, Type %d\n",
                current_mem_range.base,
                current_mem_range.limit,
                (current_mem_range.limit - current_mem_range.base) / 1024,
                current_mem_range.type);
    #endif

    if(i == memory_map_size)
    {
        return OS_ERR_NO_MORE_FREE_MEM;
    }

    init = 1;

    return OS_NO_ERR;
}

OS_RETURN_E enable_paging(void)
{
    return OS_NO_ERR;
}

OS_RETURN_E disable_paging(void)
{
    return OS_NO_ERR;
}
