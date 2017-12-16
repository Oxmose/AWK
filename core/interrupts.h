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

struct cpu_state
{
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
        unsigned int esi;
        unsigned int edi;
        unsigned int ebp;
        unsigned int esp;
} __attribute__((packed));
typedef struct cpu_state cpu_state_t;

struct stack_state 
{
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));
typedef struct stack_state stack_state_t;

void kernel_interrupt_handler(cpu_state_t cpu_state,
                              unsigned int int_id,
                              stack_state_t stack_state);

#endif /* __INTERRUPTS_H_ */