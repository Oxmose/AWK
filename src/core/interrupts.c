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
#include "../drivers/pic.h"      /* set_IRQ_PIC_EOI, set_IRQ_PIC_mask */
#include "../drivers/pit.h"      /* upadte_pit_tick */
#include "../drivers/io_apic.h"  /* set_IRQ_IO_APIC_mask */
#include "../cpu/cpu_settings.h" /* IDT_ENTRY_COUNT */
#include "../cpu/cpu.h"          /* sti cli */
#include "../cpu/lapic.h"        /* set_INT_LAPIC_EOI */
#include "../sync/lock.h"        /* enable_interrupt, disable_interrupt */
#include "kernel_output.h"       /* kernel_success */
#include "panic.h"               /* panic, interrupt */
#include "acpi.h"                /* acpi_get_io_apic_available */

#include "../debug.h"      /* kernel_serial_debug */

/* Header file */
#include "interrupts.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Handlers for each interrupt */
static custom_handler_t kernel_interrupt_handlers[IDT_ENTRY_COUNT];
static lock_t           handler_table_lock;

/* Tells the kernel if we use PIC or IO-APIC to manage IRQs */
static uint8_t io_apic_capable;

/* Tells the kernel if the LAPIC is available */
static uint8_t lapic_capable;

/* Keep track on the nexting level, kernel start with interrupt disabled */
static uint32_t int_lock_nesting = 1;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void spurious_handler(cpu_state_t* cpu, uint32_t id,
                             stack_state_t* stack)
{
    (void)cpu;
    (void)stack;

    #ifdef DEBUG_INTERRUPT
    kernel_serial_debug("Spurious interrupt\n");
    #endif

    set_IRQ_EOI(id);
    return;
}

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

OS_RETURN_E init_kernel_interrupt(void)
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

    /* Attach spurious event handler */
    kernel_interrupt_handlers[SPURIOUS_INTERRUPT_LINE].enabled = 1;
    kernel_interrupt_handlers[SPURIOUS_INTERRUPT_LINE].handler =
                                                               spurious_handler;

    /* Get IO-APIC availability */
    io_apic_capable = acpi_get_io_apic_available();

    #ifdef DEBUG_INTERRUPT
    kernel_serial_debug("Interrupt IO-APIC available: %d\n", io_apic_capable);
    #endif

    /* Get LAPIC availability */
    /* TODO */
    lapic_capable = 0;

    #ifdef DEBUG_INTERRUPT
    kernel_serial_debug("Interrupt LAPIC TIMER available: %d\n", lapic_capable);
    #endif

    spinlock_init(&handler_table_lock);

    return OS_NO_ERR;
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
       interrupt_line > MAX_INTERRUPT_LINE)
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

    #ifdef DEBUG_INTERRUPT
    kernel_serial_debug("Added INT %d handler at 0x%08x\n",
                         interrupt_line, (uint32_t)handler);
    #endif

    spinlock_unlock(&handler_table_lock);

    return OS_NO_ERR;
}

OS_RETURN_E remove_interrupt_handler(const uint32_t interrupt_line)
{
    if(interrupt_line < MIN_INTERRUPT_LINE ||
       interrupt_line > MAX_INTERRUPT_LINE)
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

    #ifdef DEBUG_INTERRUPT
    kernel_serial_debug("Removed INT %d handle\n", interrupt_line);
    #endif

    spinlock_unlock(&handler_table_lock);

    return OS_NO_ERR;
}

void enable_interrupt(void)
{
    if(int_lock_nesting > 0)
    {
        --int_lock_nesting;
    }
    if(int_lock_nesting == 0)
    {
        #ifdef DEBUG_INTERRUPT
        kernel_serial_debug("Enabled INT\n");
        #endif

        sti();
    }
}

void disable_interrupt(void)
{
    if(int_lock_nesting == 0)
    {
        #ifdef DEBUG_INTERRUPT
        kernel_serial_debug("Disabled INT\n");
        #endif

        cli();
    }
    ++int_lock_nesting;
}

OS_RETURN_E set_IRQ_mask(const uint32_t irq_number, const uint8_t enabled)
{
    OS_RETURN_E err;

    if(io_apic_capable == 1)
    {
        #ifdef DEBUG_INTERRUPT
        kernel_serial_debug("IO_APIC mask INT %d: %d\n", irq_number, enabled);
        #endif

        return set_IRQ_IO_APIC_mask(irq_number, enabled);
    }
    else
    {
        if(irq_number > 7 && enabled == 1)
        {
            err = set_IRQ_mask(PIC_CASCADING_IRQ, 1);
        }
        if(err != OS_NO_ERR)
        {
            return err;
        }

        #ifdef DEBUG_INTERRUPT
        kernel_serial_debug("PIC mask INT %d: %d\n", irq_number, enabled);
        #endif

        return set_IRQ_PIC_mask(irq_number, enabled);
    }
}

OS_RETURN_E set_IRQ_EOI(const uint32_t irq_number)
{
    if(io_apic_capable == 1)
    {
        return set_INT_LAPIC_EOI(irq_number);
    }
    else
    {
        return set_IRQ_PIC_EOI(irq_number);
    }
}

void update_tick(void)
{
    if(lapic_capable == 1)
    {
        /* TODO */
    }
    else
    {
        update_pit_tick();
    }
}

int32_t get_IRQ_SCHED_TIMER(void)
{
    if(lapic_capable == 1)
    {
        return LAPIC_TIMER_INTERRUPT_LINE;
    }
    else
    {
        return PIT_IRQ_LINE;
    }
}

int32_t get_line_SCHED_HW(void)
{
    if(lapic_capable == 1)
    {
        return LAPIC_TIMER_INTERRUPT_LINE;
    }
    else
    {
        return PIT_INTERRUPT_LINE;
    }
}


uint32_t get_current_uptime(void)
{
    if(lapic_capable == 1)
    {
        /* TODO */
        return 0;
    }
    else
    {
        return get_pit_current_uptime();
    }
}

uint32_t get_tick_count(void)
{
    if(lapic_capable == 1)
    {
        /* TODO */
        return 0;
    }
    else
    {
        return get_pit_tick_count();
    }
}