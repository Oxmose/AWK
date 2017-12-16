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
#define PIT_FREQ           100
#define PIT_IRQ            0
#define PIT_INTERRUPT_LINE 32

/* System pit events settings */
#define PIT_MAX_EVENT_COUNT 16

/*****************************************
 * STRUCTURES
 ****************************************/

/* Main pit event structure */
struct pit_event
{
    void        (*execute)(void);
    uint32_t    period;
    OS_EVENT_ID event_id;

    uint8_t enabled;
};
typedef struct pit_event pit_event_t;

/************************************
 * FUNCTIONS 
 ***********************************/

/* Clock tick handler, should only be called in an interrupt context */
void pit_interrupt_handler(cpu_state_t *cpu_state, uint32_t int_id, 
                           stack_state_t *stack_state);

/* Init pit settings
 *
 * @returns The error or success state.
 */
OS_RETURN_E init_pit(void);

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

/* Register a new event to execute on pit tick on a defined period given as
 * parameter.
 *
 * @param function The routine to execute when the period is reached.
 * @param period The period at wich the event should be executed.
 * @param event_id the OS_EVENT_ID buffer to receive the event id, may be -1 on
 * error.
 * @returns The error code. 
 */
OS_RETURN_E register_pit_event(void (*function)(void), 
                                 const uint32_t period,
                                 OS_EVENT_ID *event_id);

/* Unregister a pit event based on the event id given as parameter.
 *
 * @param event_id The even id to be unregistered.
 * @returns The error code. 
 */
OS_RETURN_E unregister_pit_event(const OS_EVENT_ID event_id);

#endif /* __PIT_H_ */