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

/* Header include */
#include "pit.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Uptime in miliseconds */
static uint32_t uptime;

/* Tick counter, circular when reaching overflow */
static uint32_t tick_count;

/* Keep track on the frequency */
static uint32_t tick_freq;

/* Keep track on the PIT state */
static uint32_t disabled_nesting;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* PIT IRQ handler. dummy routine setting EOI.
 *
 * @param cpu_state The cpu registers before the interrupt.
 * @param int_id The interrupt line that called the handler.
 * @param stack_state The stack state before the interrupt.
 */
#include "../core/kernel_output.h"
static void dummy_handler(cpu_state_t* cpu_state, uint32_t int_id,
                          stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    kernel_serial_debug("TICK");
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

    disabled_nesting = 0;

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
    /* Check if we can enable */
    if(disabled_nesting > 0)
    {
        --disabled_nesting;
    }
    if(disabled_nesting == 0)
    {
        return set_IRQ_mask(PIT_IRQ_LINE, 1);
    }

    return OS_NO_ERR;
}

OS_RETURN_E disable_pit(void)
{
    if(disabled_nesting < UINT32_MAX)
    {
        ++disabled_nesting;
    }

    return set_IRQ_mask(PIT_IRQ_LINE, 0);
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

    tick_freq  = freq;

    /* Set clock frequency */
    uint16_t tick_freq = (uint16_t)((uint32_t)PIT_QUARTZ_FREQ / freq);
    outb(PIT_COMM_SET_FREQ, PIT_COMM_PORT);
    outb(tick_freq & 0x00FF, PIT_DATA_PORT);
    outb(tick_freq >> 8, PIT_DATA_PORT);

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

    /* Remove the current handler */
    err = remove_interrupt_handler(PIT_INTERRUPT_LINE);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = register_interrupt_handler(PIT_INTERRUPT_LINE, handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    return enable_pit();
}

OS_RETURN_E remove_pit_handler(void)
{
    return set_pit_handler(dummy_handler);
}