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

/* Header file */
#include "interrupts.h"

#include "kernel_output.h"

void kernel_interrupt_handler(cpu_state_t cpu_state,
                              unsigned int int_id,
                              stack_state_t stack_state)
{
	(void)cpu_state;
	(void)stack_state;
	kernel_print("INT ", 4);
	print_unsigned(int_id);
	kernel_print("  -  ", 5);
}