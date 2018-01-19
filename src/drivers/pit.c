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

#include "../cpu/cpu.h"            /* outb */
#include "../core/interrupts.h"    /* register_interrupt, cpu_state,
                                    * stack_state, set_IRQ_mask, set_IRQ_EOI */
#include "../lib/stdint.h"         /* Generioc int types */
#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../sync/lock.h"          /* spinlock */

#include "../debug.h"      /* kernel_serial_debug */

/* Header include */
#include "pit.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Uptime in miliseconds */
static volatile uint32_t uptime;

/* Tick counter, circular when reaching overflow */
static volatile uint32_t tick_count;

/* Keep track on the frequency */
static volatile uint32_t tick_freq;

/* Keep track on the PIT state */
static volatile uint32_t disabled_nesting;

static lock_t pit_lock;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* PIT IRQ handler. dummy routine setting EOI.
 *
 * @param cpu_state The cpu registers before the interrupt.
 * @param int_id The interrupt line that called the handler.
 * @param stack_state The stack state before the interrupt.
 */
static void dummy_handler(cpu_state_t* cpu_state, uint32_t int_id,
                          stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    /* DUMMY */
    set_IRQ_EOI(PIT_IRQ_LINE);
}

OS_RETURN_E init_pit(void)
{
    OS_RETURN_E err;

    /* Init system times */
    uptime     = 0;
    tick_count = 0;
    tick_freq  = PIT_INIT_FREQ;

    disabled_nesting = 1;

    err = set_pit_freq(PIT_INIT_FREQ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Set PIT interrupt handler */
    err = register_interrupt_handler(PIT_INTERRUPT_LINE, dummy_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    spinlock_init(&pit_lock);

    /* Enable PIT IRQ */
    return enable_pit();
}

void update_pit_tick(void)
{
    ++tick_count;
    uptime += 1000 / tick_freq;
}

uint32_t get_pit_tick_count(void)
{
    return tick_count;
}

uint32_t get_pit_current_uptime(void)
{
    return uptime;
}

OS_RETURN_E enable_pit(void)
{
    spinlock_lock(&pit_lock);

    if(disabled_nesting > 0)
    {
        --disabled_nesting;
    }
    if(disabled_nesting == 0)
    {
        #ifdef DEBUG_PIT
        kernel_serial_debug("Enable PIT\n");
        #endif

        spinlock_unlock(&pit_lock);
        return set_IRQ_mask(PIT_IRQ_LINE, 1);
    }

    spinlock_unlock(&pit_lock);
    return OS_NO_ERR;
}

OS_RETURN_E disable_pit(void)
{
    OS_RETURN_E err;

    spinlock_lock(&pit_lock);

    if(disabled_nesting < UINT32_MAX)
    {
        ++disabled_nesting;
    }

    #ifdef DEBUG_PIT
    kernel_serial_debug("Disable PIT (%d)\n", disabled_nesting);
    #endif
    err = set_IRQ_mask(PIT_IRQ_LINE, 0);

    spinlock_unlock(&pit_lock);

    return err;
}

OS_RETURN_E set_pit_freq(const uint32_t freq)
{
    OS_RETURN_E err;

    if(freq < PIT_MIN_FREQ || freq > PIT_MAX_FREQ)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Disable PIT IRQ */
    err = disable_pit();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    spinlock_lock(&pit_lock);

    tick_freq  = freq;

    /* Set clock frequency */
    uint16_t tick_freq = (uint16_t)((uint32_t)PIT_QUARTZ_FREQ / freq);
    outb(PIT_COMM_SET_FREQ, PIT_COMM_PORT);
    outb(tick_freq & 0x00FF, PIT_DATA_PORT);
    outb(tick_freq >> 8, PIT_DATA_PORT);

    #ifdef DEBUG_PIT
    kernel_serial_debug("New PIT frequency set (%d)\n", freq);
    #endif

    spinlock_unlock(&pit_lock);

    /* Enable PIT IRQ */
    return enable_pit();
}

OS_RETURN_E set_pit_handler(void(*handler)(
                                 cpu_state_t*,
                                 uint32_t,
                                 stack_state_t*
                                 ))
{
    OS_RETURN_E err;

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = disable_pit();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    spinlock_lock(&pit_lock);

    /* Remove the current handler */
    err = remove_interrupt_handler(PIT_INTERRUPT_LINE);
    if(err != OS_NO_ERR)
    {
        spinlock_unlock(&pit_lock);
        enable_pit();
        return err;
    }

    err = register_interrupt_handler(PIT_INTERRUPT_LINE, handler);
    if(err != OS_NO_ERR)
    {
        spinlock_unlock(&pit_lock);
        enable_pit();
        return err;
    }

    #ifdef DEBUG_PIT
    kernel_serial_debug("New PIT handler set (0x%08x)\n", handler);
    #endif

    spinlock_unlock(&pit_lock);
    return enable_pit();
}

OS_RETURN_E remove_pit_handler(void)
{
    #ifdef DEBUG_PIT
    kernel_serial_debug("Default PIT handler set\n");
    #endif
    return set_pit_handler(dummy_handler);
}
