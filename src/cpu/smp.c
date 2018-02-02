/*******************************************************************************
 *
 * File: smp.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 31/12/2017
 *
 * Version: 1.0
 *
 * SMP (Simetric MultiProcessing) management for the Kernel
 * Mabye will do an AMP version later that could be fun too
 ******************************************************************************/

#include "../lib/stdint.h"         /* Generic int types */
#include "../drivers/acpi.h"       /* acpi data */

/* Header file */
#include "smp.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static int8_t cpu_count = -1;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

int8_t get_cpu_count(void)
{
    /* Detect the number of CPU thanks to the ACPI tables */
    if(cpu_count == -1)
    {
        cpu_count = acpi_get_detected_cpu_count();
    }

    return cpu_count;
}
