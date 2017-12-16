/*******************************************************************************
 *
 * File: panic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.0
 *
 * Kernel panic functions. Displays the CPU registers, the faulty instruction
 * and the interrupt ID and cause.
 ******************************************************************************/

#ifndef __PANIC_H_
#define __PANIC_H_

#include "interrupts.h" /* cpu_state_t, stack_state_t */
#include "../lib/stdint.h" /* Generic int types */

/* Display the kernel panic screen. This sreen dump the CPU registers and the 
 * stack state before the panis occurs (panic is usually called by interrupts).
 * @param cpu_state The cpu registers structure.
 * @param int_id The interrupt number, -1 if panis is called by an regular code.
 * @param stack_state The stack state before the interrupt that contain cs, eip, 
 * error code and the eflags register value.
 */
void panic(cpu_state_t *cpu_state, uint32_t int_id, stack_state_t *stack_state);

#endif /* __PANIC_H_ */