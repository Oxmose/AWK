/*******************************************************************************
 *
 * File: rtc.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * System RTC (Real Time Clock) driver.
 ******************************************************************************/

#include "../cpu/cpu.h"            /* outb inb */
#include "../core/interrupts.h"    /* register_interrupt, cpu_state,
                                    * stack_state, set_IRQ_EOI, set_IRQ_mask */
#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/stddef.h"         /* OS_RETURN_E, OS_EVENT_ID */
#include "../sync/lock.h"          /* spinlock */

#include "../debug.h"      /* kernel_serial_debug */

/* Header file*/
#include "rtc.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Real day time in seconds */
static uint32_t day_time;

/* System date */
static date_t date;

/* Events table */
static rtc_event_t clock_events[RTC_MAX_EVENT_COUNT];
static lock_t      clock_events_lock;

/* Tick count */
static volatile uint32_t tick_count;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void update_time(void)
{
    int8_t nmi_info;
    uint8_t seconds;
    uint8_t minutes;
    uint32_t hours;
    uint8_t century;
    uint8_t reg_b;

    /* Set NMI info bit */
    nmi_info = CMOS_NMI_DISABLE_BIT << 7;

    /* Set time */
    /* Select CMOS seconds register and read */
    outb(nmi_info | CMOS_SECONDS_REGISTER, CMOS_COMM_PORT);
    seconds = inb(CMOS_DATA_PORT);

    /* Select CMOS minutes register and read */
    outb(nmi_info | CMOS_MINUTES_REGISTER, CMOS_COMM_PORT);
    minutes = inb(CMOS_DATA_PORT);

    /* Select CMOS hours register and read */
    outb(nmi_info | CMOS_HOURS_REGISTER, CMOS_COMM_PORT);
    hours = inb(CMOS_DATA_PORT);

    /* Set date */

    /* Select CMOS day register and read */
    outb(nmi_info | CMOS_DAY_REGISTER, CMOS_COMM_PORT);
    date.day = inb(CMOS_DATA_PORT);

    /* Select CMOS month register and read */
    outb(nmi_info | CMOS_MONTH_REGISTER, CMOS_COMM_PORT);
    date.month = inb(CMOS_DATA_PORT);

    /* Select CMOS years register and read */
    outb(nmi_info | CMOS_YEAR_REGISTER, CMOS_COMM_PORT);
    date.year = inb(CMOS_DATA_PORT);

    /* Select CMOS century register and read */

    if(CMOS_CENTURY_REGISTER != 0)
    {
        outb(nmi_info | CMOS_CENTURY_REGISTER, CMOS_COMM_PORT);
        century = inb(CMOS_DATA_PORT);
    }

    /* Convert BCD to binary if necessary */
    outb(CMOS_REG_B, CMOS_COMM_PORT);
    reg_b = inb(CMOS_DATA_PORT);

    if((reg_b & 0x04) == 0)
    {
        seconds = (seconds & 0x0F) + ((seconds / 16) * 10);
        minutes = (minutes & 0x0F) + ((minutes / 16) * 10);
        hours = ((hours & 0x0F) + (((hours & 0x70) / 16) * 10)) |
            (hours & 0x80);
        date.day = (date.day & 0x0F) + ((date.day / 16) * 10);
        date.month = (date.month & 0x0F) + ((date.month / 16) * 10);
        date.year = (date.year & 0x0F) + ((date.year / 16) * 10);

        if(CMOS_CENTURY_REGISTER != 0)
        {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    /*  Convert to 24H */
    if((reg_b & 0x02) == 0 && (hours & 0x80) == 1)
    {
        hours = ((hours & 0x7F) + 12) % 24;
    }

    /* Get year */
    if(CMOS_CENTURY_REGISTER != 0)
    {
        date.year += century * 100;
    }
    else
    {
        date.year = date.year + 2000;
    }

    /* Compute week day and day time */
    date.weekday = ((date.day + date.month + date.year + date.year / 4)
            + 1) % 7 + 1;
    day_time = seconds + 60 * minutes + 3600 * hours;

    /* Clear C Register */
    outb(CMOS_REG_C, CMOS_COMM_PORT);
    inb(CMOS_DATA_PORT);
}

static void rtc_interrupt_handler(cpu_state_t *cpu_state, uint32_t int_id,
                                  stack_state_t *stack_state)
{
    uint32_t    i;

    (void)cpu_state;
    (void)stack_state;
    (void)int_id;

    ++tick_count;

    update_time();

    /* Execute events */
    for(i = 0; i < RTC_MAX_EVENT_COUNT; ++i)
    {
        /* Check update frequency */
        if(clock_events[i].enabled  == 1 &&
           clock_events[i].execute != NULL &&
           tick_count % clock_events[i].period == 0)
        {
            clock_events[i].execute();
        }
    }

    /* Send EOI signal */
    set_IRQ_EOI(RTC_IRQ_LINE);
}

OS_RETURN_E init_rtc(void)
{

    OS_RETURN_E err;
    int8_t      prev_ored;
    int8_t      prev_rate;
    uint32_t    i;

    /* Init real times */
    day_time     = 0;
    date.weekday = 0;
    date.day     = 0;
    date.month   = 0;
    date.year    = 0;

    spinlock_init(&clock_events_lock);

    /* Init CMOS IRQ8 */
    outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_B, CMOS_COMM_PORT);
    prev_ored = inb(CMOS_DATA_PORT);
    outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_B, CMOS_COMM_PORT);
    outb(prev_ored | CMOS_ENABLE_RTC, CMOS_DATA_PORT);

    /* Init CMOS IRQ8 rate */
    outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_A, CMOS_COMM_PORT);
    prev_rate = inb(CMOS_DATA_PORT);
    outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_A, CMOS_COMM_PORT);
    outb((prev_rate & 0xF0) | RTC_RATE, CMOS_DATA_PORT);

    tick_count = 0;

    for(i = 0; i < RTC_MAX_EVENT_COUNT; ++i)
    {
        clock_events[i].enabled = 0;
    }

    /* Set handler and mask before setting IRQ */
    /* Set rtc clock interrupt handler */
    err = register_interrupt_handler(RTC_INTERRUPT_LINE,
                                     rtc_interrupt_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = set_IRQ_mask(RTC_IRQ_LINE, 1);

    /* Just dummy read register C to unlock interrupt */
    outb(CMOS_REG_C, CMOS_COMM_PORT);
    inb(CMOS_DATA_PORT);

    return err;
}

uint32_t get_current_daytime(void)
{
    return day_time;
}

date_t get_current_date(void)
{
    return date;
}

OS_RETURN_E register_rtc_event(void (*function)(void),
                               const uint32_t period,
                               OS_EVENT_ID* event_id)
{
    uint32_t i;
    if(function == NULL)
    {
        if(event_id != NULL)
        {
            *event_id = -1;
        }
        return OS_ERR_NULL_POINTER;
    }

    spinlock_lock(&clock_events_lock);

    /* Search for free event id */
    for(i = 0; i < RTC_MAX_EVENT_COUNT && clock_events[i].enabled == 1; ++i);

    if(i == RTC_MAX_EVENT_COUNT)
    {
        if(event_id != NULL)
        {
            *event_id = -1;
        }
        spinlock_unlock(&clock_events_lock);
        return OS_ERR_NO_MORE_FREE_EVENT;
    }

    /* Create new event */
    clock_events[i].execute  = function;
    clock_events[i].period   = period;
    clock_events[i].event_id = i;
    clock_events[i].enabled  = 1;

    if(event_id != NULL)
    {
        *event_id = i;
    }

    #ifdef DEBUG_RTC
    kernel_serial_debug("Registered RTC event id %d, handler 0x%08x\n",
                         i, (uint32_t)function);
    #endif

    spinlock_unlock(&clock_events_lock);

    return OS_NO_ERR;
}

OS_RETURN_E unregister_rtc_event(const OS_EVENT_ID event_id)
{
    if(event_id >= RTC_MAX_EVENT_COUNT)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    spinlock_lock(&clock_events_lock);

    if(clock_events[event_id].enabled == 0)
    {
        spinlock_unlock(&clock_events_lock);
        return OS_ERR_NO_SUCH_ID;
    }

    /* Disable event */
    clock_events[event_id].enabled = 0;
    clock_events[event_id].execute = NULL;
    clock_events[event_id].period  = 0;

    #ifdef DEBUG_RTC
    kernel_serial_debug("Unregistered RTC event id %d\n", event_id);
    #endif

    spinlock_unlock(&clock_events_lock);

    return OS_NO_ERR;
}
