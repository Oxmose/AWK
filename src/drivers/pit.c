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
 * PIT (Programmable interval timer) driver
 * System main timer management. Allows to define system ticks.
 * The PIT is only used to call the scheduler.
 *
 ******************************************************************************/

#include "../cpu/cpu.h"            /* outb */
#include "../core/interrupts.h"    /* register_interrupt, cpu_state, 
                                    * stack_state, set_IRQ_mask, set_IRQ_EOI */
#include "../lib/stdint.h"         /* Generioc int types */
#include "../lib/stddef.h"         /* OS_RETURN_E */

/* Header include */
#include "pit.h"

/* Uptime in miliseconds */
static uint32_t uptime;

/* Tick counter, circular when reaching overflow */
static uint32_t tick_count;

static void dummy_handler(cpu_state_t *cpu_state, uint32_t int_id, 
                          stack_state_t *stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;
    /* DUMMY */
    set_IRQ_EOI(PIT_IRQ);
}

OS_RETURN_E init_pit(void)
{
    OS_RETURN_E err;
    
    /* Init system times */
    uptime = 0;
    tick_count = 0;

    /* Set clock frequency */
    uint16_t tick_freq = (uint16_t)((uint32_t)PIT_QUARTZ_FREQ / PIT_FREQ);
    outb(PIT_COMM_SET_FREQ, PIT_COMM_PORT);
    outb(tick_freq & 0x00FF, PIT_DATA_PORT);
    outb(tick_freq >> 8, PIT_DATA_PORT);

    /* Set clock interrupt handler */
    err = register_interrupt_handler(PIT_INTERRUPT_LINE, 
                                     dummy_handler);

    if(err == OS_NO_ERR)
    {
        /* Set IRQ mask for system clock */
        err = set_IRQ_mask(PIT_IRQ, 1);
    }

    return err;
}

void update_tick(void)
{
    ++tick_count;
    uptime += 1000 / PIT_FREQ;
}

uint32_t get_tick_count(void)
{
    return tick_count;
}

uint32_t get_current_uptime(void)
{
    return uptime;
}