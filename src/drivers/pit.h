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
 * PIT (Programmable interval timer) driver.
 ******************************************************************************/

#ifndef __PIT_H_
#define __PIT_H_

#include "../lib/stdint.h"            /* Generioc int types */
#include "../lib/stddef.h"            /* OS_RETURN_E, OS_EVENT_ID*/
#include "../core/interrupts.h"       /* Interrupts handler prototype */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* System pit settings */
#define PIT_QUARTZ_FREQ    0x1234DD
#define PIT_COMM_PORT      0x34
#define PIT_DATA_PORT      0x40
#define PIT_COMM_SET_FREQ  0x43
#define PIT_INIT_FREQ      20
#define PIT_MIN_FREQ       20
#define PIT_MAX_FREQ       8000

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init PIT settings and enable interrupts for the PIT.
 *
 * @returns The error or success state.
 */
OS_RETURN_E init_pit(void);

/* Uptade tick count and uptime */
void update_pit_tick(void);

/* Return current uptime
 *
 * @returns The current uptime in seconds.
 */
uint32_t get_pit_current_uptime(void);

/* Return tick count since the system started
 *
 * @returns Tick count since the system started
 */
uint32_t get_pit_tick_count(void);

/* Enables PIT ticks by clearing the IRQ mask.
 *
 * @return OS_NO_ERR is returned is successful, an error is returned otherwise.
 */
OS_RETURN_E enable_pit(void);

/* Disables PIT ticks by seting the IRQ mask
 *
 * @return OS_NO_ERR is returned is successful, an error is returned otherwise.
 */
OS_RETURN_E disable_pit(void);

/* Set the PIT tick frequency, the value must be between 20Hz and 8000Hz
 *
 * @param freq The new frequency to set to the PIT.
 * @return OS_NO_ERR is returned is successful, an error is returned otherwise.
 */
OS_RETURN_E set_pit_freq(const uint32_t freq);

/* Set the PIT tick handler.
 *
 * @param handler The handler for the PIT interrupt.
 * @return OS_NO_ERR is returned is successful, an error is returned otherwise.
 */
OS_RETURN_E set_pit_handler(void(*handler)(
                                 cpu_state_t*,
                                 uint32_t,
                                 stack_state_t*
                                 ));

/* Remove the current PIT interrupt handler.
 *
 * @return OS_NO_ERR is returned is successful, an error is returned otherwise.
 */
OS_RETURN_E remove_pit_handler(void);

#endif /* __PIT_H_ */
