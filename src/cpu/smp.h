/*******************************************************************************
 *
 * File: smp.h
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

#ifndef __SMP_H_
#define __SMP_H_

#include "../lib/stdint.h" /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define MAX_CPU_COUNT 32

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Returns the number of CPU detected on the system. -1 is returned in case of
 * error.
 *
 * @returns The number of CPU detected in the system, -1 on error.
 */
int8_t get_cpu_count(void);

#endif /* __SMP_H_ */
