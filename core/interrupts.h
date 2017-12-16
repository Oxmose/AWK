/*******************************************************************************
 *
 * File: interrupts.h
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

#ifndef __INTERRUPTS_H_
#define __INTERRUPTS_H_

/* Generic int types */
#include "../lib/stdint.h"

/* Holds the CPU register values */
struct cpu_state
{
    uint32_t esp;
    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;

    uint32_t ss;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
} __attribute__((packed));
typedef struct cpu_state cpu_state_t;

/* Hold the stack state before the interrupt */
struct stack_state 
{
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed));
typedef struct stack_state stack_state_t;

/* Generic and global interrupt handler. This function should only be called
 * by an assembly interrupt handler. The function will dispatch the interrupt
 * to the desired function to handler the interrupt.
 * @param cpu_state The cpu registers structure.
 * @param int_id The interrupt number.
 * @param stack_state The stack state before the interrupt that contain cs, eip, 
 * error code and the eflags register value.
 */
void kernel_interrupt_handler(cpu_state_t cpu_state,
                              uint32_t int_id,
                              stack_state_t stack_state);

#endif /* __INTERRUPTS_H_ */