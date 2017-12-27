/*******************************************************************************
 *
 * File: mouse.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.5
 *
 * Mouse driver (PS2) for the kernel.
 ******************************************************************************/

#ifndef __MOUSE_H_
#define __MOUSE_H_

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E */

/*********************************
 * CONSTANTS 
 ********************************/

/* Mouse settings */
#define MOUSE_COMM_PORT      0x64
#define MOUSE_DATA_PORT      0x60

#define MOUSE_INTERRUPT_LINE_PS2 44
#define MOUSE_IRQ_PS2            12

#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

#define MOUSE_LEFT_CLICK   0x01
#define MOUSE_RIGHT_CLICK  0x02
#define MOUSE_MIDDLE_CLICK 0x04

#define MOUSE_MAGIC 0xFEED1234

#define MOUSE_MAX_EVENT_COUNT 32

/*********************************
 * STRUCTURES
 ********************************/
typedef struct mouse_state
{
	int32_t pos_x; /* Relative position compared to the previous position */
	int32_t pos_y; /* Relative position compared to the previous position */

	uint32_t flags;
} mouse_state_t;

typedef struct mouse_event
{
    void        (*execute)(void);
    OS_EVENT_ID event_id;

    uint8_t     enabled;
} mouse_event_t;

/********************************
 * FUNCTIONS
 *******************************/

/* Init mouse settings
 *
 * @return The error or success state.
 */
OS_RETURN_E init_mouse(void);

/* Register a new event to execute on mouse interrupt.
 *
 * @param function The routine to execute when the period is reached.
 * @param event_id the OS_EVENT_ID buffer to receive the event id, may be -1 on
 * error.
 * @returns The error code. 
 */
OS_RETURN_E register_mouse_event(void (*function)(void),
                                 OS_EVENT_ID *event_id);

/* Unregister a mouse event based on the event id given as parameter.
 *
 * @param event_id The even id to be unregistered.
 * @returns The error code. 
 */
OS_RETURN_E unregister_mouse_event(const OS_EVENT_ID event_id);

OS_RETURN_E get_mouse_state(mouse_state_t *state);

#endif /* __MOUSE_H_ */
