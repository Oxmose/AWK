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

#include "../lib/stdint.h"       /* Generic int types */
#include "../lib/stddef.h"       /* OS_RETURN_E */
#include "../lib/string.h"       /* memset */
#include "../cpu/cpu_settings.h" /* IDT_ENTRY_COUNT */
#include "../sync/lock.h"        /* enable_interrupt, disable_interrupt */
#include "kernel_output.h"       /* kernel_success */
#include "panic.h"               /* panic, interrupt */


/* Header file */
#include "interrupts.h"

/* Handlers for each interrupt */
static custom_handler_t kernel_interrupt_handlers[IDT_ENTRY_COUNT];
static lock_t handler_table_lock;

/*******************************************************************************
 * GLOBAL INTERRUPT HANDLER 
 ******************************************************************************/
void kernel_interrupt_handler(cpu_state_t cpu_state,
                              uint32_t int_id,
                              stack_state_t stack_state)
{
   /* Execute custom handlers */
    if(int_id < IDT_ENTRY_COUNT && 
     kernel_interrupt_handlers[int_id].enabled == 1 &&
     kernel_interrupt_handlers[int_id].handler != NULL)
    {
        kernel_interrupt_handlers[int_id]
        .handler(&cpu_state, int_id, &stack_state);
    }
    else
    {
        panic(&cpu_state, int_id, &stack_state);
    }
}

void init_kernel_interrupt(void)
{
    uint32_t i;

    /* Blank custo interrupt handlers */
    memset(kernel_interrupt_handlers, 0, 
           sizeof(custom_handler_t) * IDT_ENTRY_COUNT);
  
    /* Attach Panic to the first 32 interrupt */
    for(i = 0; i < 32; ++i)
    {
        kernel_interrupt_handlers[i].enabled = 1;
        kernel_interrupt_handlers[i].handler = panic;
    }

    /* Attach the special PANIC interrupt for when we donMt know what to do */
    kernel_interrupt_handlers[PANIC_INT_LINE].enabled = 1;
    kernel_interrupt_handlers[PANIC_INT_LINE].handler = panic;

    spinlock_init(&handler_table_lock);

    kernel_success("KIH Initialized\n");
}

OS_RETURN_E register_interrupt_handler(const uint32_t interrupt_line, 
                                       void(*handler)(
                                             cpu_state_t*, 
                                             uint32_t, 
                                             stack_state_t*
                                             )
                                       )
{
    if(interrupt_line < MIN_INTERRUPT_LINE || 
       interrupt_line >= MAX_INTERRUPT_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    spinlock_lock(&handler_table_lock);

    if(kernel_interrupt_handlers[interrupt_line].handler != NULL)
    {
        spinlock_unlock(&handler_table_lock);
        return OS_ERR_INTERRUPT_ALREADY_REGISTERED;
    }

    kernel_interrupt_handlers[interrupt_line].handler = handler;
    kernel_interrupt_handlers[interrupt_line].enabled = 1;

    spinlock_unlock(&handler_table_lock);

    return OS_NO_ERR;
}

OS_RETURN_E remove_interrupt_handler(const uint32_t interrupt_line)
{
    if(interrupt_line < MIN_INTERRUPT_LINE || 
       interrupt_line >= MAX_INTERRUPT_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }
    
    spinlock_lock(&handler_table_lock);

    if(kernel_interrupt_handlers[interrupt_line].handler == NULL)
    {
        spinlock_unlock(&handler_table_lock);
        return OS_ERR_INTERRUPT_NOT_REGISTERED;
    }

    kernel_interrupt_handlers[interrupt_line].handler = NULL;
    kernel_interrupt_handlers[interrupt_line].enabled = 0;

    spinlock_unlock(&handler_table_lock);

    return OS_NO_ERR;
}