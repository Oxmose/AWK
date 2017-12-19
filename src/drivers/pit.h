/*******************************************************************************
 *
 * File: pit.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * PIT (Programmable interval timer) driver
 * System main timer management. Allows to define system ticks.
 * The PIT is only used to call the scheduler.
 *
 ******************************************************************************/

#ifndef __PIT_H_
#define __PIT_H_

#include "../lib/stdint.h"      /* Generioc int types */
#include "../lib/stddef.h"      /* OS_RETURN_E, OS_EVENT_ID*/
#include "../core/interrupts.h" /* register_interrupt, cpu_state, stack_state */

/*****************************************
 * CONSTANTS
 ****************************************/

/* System pit settings */
#define PIT_QUARTZ_FREQ    0x1234DD
#define PIT_COMM_PORT      0x34
#define PIT_DATA_PORT      0x40
#define PIT_COMM_SET_FREQ  0x43
#define PIT_FREQ           1000
#define PIT_IRQ            0
#define PIT_INTERRUPT_LINE 32


/************************************
 * FUNCTIONS 
 ***********************************/
/* Init pit settings
 *
 * @returns The error or success state.
 */
OS_RETURN_E init_pit(void);

/* Uptade tick count and uptime */
void update_tick(void);

/* Return current uptime
 *
 * @returns The current uptime in seconds.
 */
uint32_t get_current_uptime(void);

/* Return tick count since the system started
 *
 * @returns Tick count since the system started
 */
uint32_t get_tick_count(void);

/* Returns tick count since the last call to this function
 *
 * @returns Tick count since the last call to this function
 */
uint32_t get_cpu_tick_count(void);

#endif /* __PIT_H_ */