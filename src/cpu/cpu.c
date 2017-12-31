/*******************************************************************************
 *
 * File: cpu.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * CPU management functions
 ******************************************************************************/

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/string.h" /* memcpy */

/* Header file */
#include "cpu.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* May not be static since is used as extern in ASM */
cpu_info_t cpu_info;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E get_cpu_info(cpu_info_t* info)
{
    if(info == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    memcpy(info, &cpu_info, sizeof(cpu_info_t));

    return OS_NO_ERR;
}

int8_t cpuid_capable(void)
{
    return ((cpu_info.cpu_flags & CPU_FLAG_CPUID_CAPABLE) >> 21) & 0x1;
}