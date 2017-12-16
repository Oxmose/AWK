/*******************************************************************************
 *
 * File: interrupts.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 14/12/2017
 *
 * Version: 1.0
 *
 * X86 interrupt manager. Allows to attach ISR to interrupt lines and
 * manage IRQ used by the CPU.
 * We also define the general interrupt handler here.
 ******************************************************************************/

/* Generic int types */
#include "../lib/stdint.h"

/* Header file */
#include "interrupts.h"

/* kernel_print_unsigned_hex, kernel_print*/
#include "kernel_output.h"

/* panic */
#include "panic.h"

void kernel_interrupt_handler(cpu_state_t cpu_state,
                              uint32_t int_id,
                              stack_state_t stack_state)
{
	/* Div by 0 */
	if(int_id == 0)
	{
		panic(cpu_state, int_id, stack_state);
	}
	else
	{
		kernel_print("\nINT ", 5);
		kernel_print_unsigned_hex(int_id, 8);
		__asm__ __volatile__("hlt");
	}
}