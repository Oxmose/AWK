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

#include "pic.h"                   /* set_IRQ_mask */
#include "../cpu/cpu.h"            /* outb */
#include "../core/interrupts.h"    /* register_interrupt, cpu_state, 
                                    * stack_state */
#include "../lib/stdint.h"         /* Generioc int types */
#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../core/scheduler.h"     /* schedule_int */

/* Header include */
#include "pit.h"

/* Uptime in miliseconds */
static uint32_t uptime;

/* Tick counter, circular when reaching overflow */
static uint32_t tick_count;

OS_RETURN_E init_pit(void)
{
    /* Init system times */
    uptime = 0;
    tick_count = 0;

    /* Set clock frequency */
    uint16_t tick_freq = (uint16_t)((uint32_t)PIT_QUARTZ_FREQ / PIT_FREQ);
    outb(PIT_COMM_SET_FREQ, PIT_COMM_PORT);
    outb(tick_freq & 0x00FF, PIT_DATA_PORT);
    outb(tick_freq >> 8, PIT_DATA_PORT);

    /* Set clock interrupt handler */
    OS_RETURN_E err = register_interrupt_handler(PIT_INTERRUPT_LINE, 
                                                 schedule_int);

    if(err == OS_NO_ERR)
    {
        /* Set IRQ mask for system clock */
        err = set_IRQ_mask(PIT_IRQ, 1);
    }

    return err;
}

uint32_t get_tick_count(void)
{
    return tick_count;
}

uint32_t get_current_uptime(void)
{
    return uptime;
}