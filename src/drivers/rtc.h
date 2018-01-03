/*******************************************************************************
 *
 * File: rtc.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * System RTC (Real Time Clock) driver.
 ******************************************************************************/

#ifndef __RTC_H_
#define __RTC_H_

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* RTC clock settings */
#define RTC_RATE            15
#define RTC_MAX_EVENT_COUNT 20

/* CMOS registers  */
#define CMOS_SECONDS_REGISTER  0x00
#define CMOS_MINUTES_REGISTER  0x02
#define CMOS_HOURS_REGISTER    0x04
#define CMOS_WEEKDAY_REGISTER  0x06
#define CMOS_DAY_REGISTER      0x07
#define CMOS_MONTH_REGISTER    0x08
#define CMOS_YEAR_REGISTER     0x09
#define CMOS_CENTURY_REGISTER  0x00

/* CMOS setings */
#define CMOS_NMI_DISABLE_BIT 1
#define CMOS_ENABLE_RTC      0x40
#define CMOS_REG_A           0x0A
#define CMOS_REG_B           0x0B
#define CMOS_REG_C           0x0C
#define CMOS_COMM_PORT       0x70
#define CMOS_DATA_PORT       0x71

#define CURRENT_YEAR 2018

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* Date structure */
typedef struct date
{
    /* Day of the week */
    uint16_t weekday;

    /* Normal date */
    uint16_t day;
    uint16_t month;
    uint16_t year;
} date_t;

/* Main clock event structure */
typedef struct rtc_event
{
    void        (*execute)(void);
    uint32_t    period;
    OS_EVENT_ID event_id;

    uint8_t     enabled;
} rtc_event_t;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init system rtc time and date
 *
 * @return The succes or error state.
 */
OS_RETURN_E init_rtc(void);

/* Return current date
 *
 * @returns The current date in the date_t structure.
 */
date_t get_current_date(void);

/* Return current daytime in seconds.
 *
 * @returns The current daytime in seconds.
 */
uint32_t get_current_daytime(void);

/* Register a new event to execute on clock tick on a defined period given as
 * parameter.
 *
 * @param function The routine to execute when the period is reached.
 * @param period The period at wich the event should be executed.
 * @param event_id the OS_EVENT_ID buffer to receive the event id, may be -1 on
 * error.
 * @returns The error code.
 */
OS_RETURN_E register_rtc_event(void (*function)(void),
                               const uint32_t period,
                               OS_EVENT_ID* event_id);

/* Unregister a clock event based on the event id given as parameter.
 *
 * @param event_id The even id to be unregistered.
 * @returns The error code.
 */
OS_RETURN_E unregister_rtc_event(const OS_EVENT_ID event_id);

#endif /* __RTC_H_ */