/*******************************************************************************
 *
 * File: lapic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/12/2017
 *
 * Version: 1.0
 *
 * Local APIC (Advanced programmable interrupt controler) driver.
 *
 ******************************************************************************/

#include "../lib/stdint.h"       /* Generic int types */
#include "../lib/stddef.h"       /* OS_RETURN_E */
#include "../core/interrupts.h"  /* SPURIOUS_INTERRUPT_LINE,
                                    register_interrupt_line */
#include "../cpu/cpu.h"          /* mapped_io_read_32, mapped_io_write_32 */
#include "acpi.h"                /* get_lapic_addr */
#include "pit.h"                 /* set_pit_freq, emable_pit, diable_pit */

#include "../debug.h"           /* kernel_serial_debug */

/* Header file */
#include "lapic.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Local APIC controller address */
static uint8_t* lapic_base_addr;

/* Lapic TIMER settings */
static volatile uint8_t  wait_int;
static volatile uint32_t lapic_timer_frequency;
static volatile uint32_t tick_count;
static volatile uint32_t uptime;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Read Local APIC register, the acces is a memory mapped IO.
 *
 * @param reg The register of the Local APIC to read.
 * @return The value contained in the Local APIC register.
 */
__inline__ static uint32_t lapic_read(uint32_t reg)
{
    return mapped_io_read_32(lapic_base_addr + reg);
}

/* Write Local APIC register, the acces is a memory mapped IO.
 *
 * @param reg The register of the Local APIC to write.
 * @param data The value to write in the register.
 */
__inline__ static void lapic_write(uint32_t reg, uint32_t data)
{
    mapped_io_write_32(lapic_base_addr + reg, data);
}

/* LAPIC dummy hamdler.
 * @param cpu_state The cpu registers structure.
 * @param int_id The interrupt number.
 * @param stack_state The stack state before the interrupt that contain cs, eip,
 * error code and the eflags register value.
 */
static void lapic_dummy_handler(cpu_state_t* cpu_state, uint32_t int_id,
                                stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    set_IRQ_EOI(LAPIC_TIMER_INTERRUPT_LINE);
}

/* Initialisation handler. The PIT will trigger two interrupts to init the
 * LAPIC timer. This is used to get the LAPIC timer frequency.
 *
 * @param cpu_state The cpu registers structure.
 * @param int_id The interrupt number.
 * @param stack_state The stack state before the interrupt that contain cs, eip,
 * error code and the eflags register value.
 */
static void lapic_init_pit_handler(cpu_state_t* cpu_state, uint32_t int_id,
                                   stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    if(wait_int == 1)
    {
        ++wait_int;
        /* Set LAPIC init counter to -1 */
        lapic_write(LAPIC_TICR, 0xFFFFFFFF);
    }
    else if(wait_int == 2)
    {
        /* Stop the LAPIC timer */
        lapic_write(LAPIC_TIMER, APIC_LVT_INT_MASKED);
        wait_int = 0;
    }

    set_IRQ_EOI(PIT_IRQ_LINE);
}

OS_RETURN_E init_lapic(void)
{
    /* Get Local APIC base address */
    lapic_base_addr = get_lapic_addr();

    /* Enable all interrupts */
    lapic_write(LAPIC_TPR, 0);

    /* Set logical destination mode */
    lapic_write(LAPIC_DFR, 0xffffffff);
    lapic_write(LAPIC_LDR, 0x01000000);

    /* Spurious Interrupt Vector Register */
    lapic_write(LAPIC_SVR, 0x100 | SPURIOUS_INT_LINE);

    return OS_NO_ERR;
}

OS_RETURN_E init_lapic_timer(void)
{
    uint32_t    lapic_timer_tick_10ms;
    uint32_t    new_count;
    OS_RETURN_E err;

    /* Init LAPIC TIMER */
    wait_int = 1;
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);

    /* Set PIT period of 10 ms and handler */
    err = set_pit_freq(100);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = set_pit_handler(lapic_init_pit_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Wait for interrupts to gather the timer data */
    err = enable_pit();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    enable_interrupt();
    while(wait_int != 0);
    disable_interrupt();

    err = disable_pit();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = remove_pit_handler();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Get the count of ticks in 10ms */
    lapic_timer_tick_10ms = 0xFFFFFFFF - lapic_read(LAPIC_TCCR);

    /* Get the frequency */
    lapic_timer_frequency = 100 * lapic_timer_tick_10ms;

    /* Compute the new tick count */
    new_count = lapic_timer_frequency / LAPIC_TIMER_SCHED_FREQUENCY;

    /* Register LAPI dummy handler */
    err = register_interrupt_handler(LAPIC_TIMER_INTERRUPT_LINE,
                                     lapic_dummy_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Init interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_INTERRUPT_LINE |
                LAPIC_TIMER_MODE_PERIODIC);

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, new_count);

    tick_count = 0;
    uptime = 0;

    return OS_NO_ERR;
}

uint32_t get_lapic_id(void)
{
    return (lapic_read(LAPIC_ID) >> 24);
}

OS_RETURN_E lapic_send_ipi_init(const uint32_t lapic_id)
{
    OS_RETURN_E err;

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, ICR_INIT | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    return err;
}

OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id,
                                   const uint32_t vector)
{
    OS_RETURN_E err;

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, vector | ICR_STARTUP | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    return err;
}

OS_RETURN_E set_INT_LAPIC_EOI(const uint32_t interrupt_line)
{
    if(interrupt_line > SPURIOUS_INT_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    lapic_write(LAPIC_EOI, 0);

    #ifdef DEBUG_LAPIC
    kernel_serial_debug("LAPIC EOI %d \n", interrupt_line);
    #endif

    return OS_NO_ERR;
}

void update_lapic_tick(void)
{
    ++tick_count;
    uptime += 1000 / LAPIC_TIMER_SCHED_FREQUENCY;
}

uint32_t get_lapic_tick_count(void)
{
    return tick_count;
}

uint32_t get_lapic_current_uptime(void)
{
    return uptime;
}
