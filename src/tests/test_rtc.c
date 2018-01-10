/*******************************************************************************
 *
 * File: test_rtc.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Real Time Clock
 ******************************************************************************/

/*
 * !!! THESE TESTS MUST BE DONE BEFORE INITIALIZING ANY INTERRUPT HANDLER
 *     BETWEEN MIN_INTERRUPT_LINE AND MAX_INTERRUPT_LINE !!!
 * !!! THESE TESTS MUST BE DONE AFTER INITIALIZING THE PIC OR THE
 *     IOAPIC!!!
 */

#include "../core/interrupts.h"
#include "../core/kernel_output.h"
#include "../core/panic.h"
#include "../cpu/cpu.h"
#include "../drivers/rtc.h"

static volatile uint32_t counter;

static void handler(void)
{
    ++counter;
    return;
}

void test_rtc(void)
{
    int32_t i;
    enable_interrupt();

    OS_EVENT_ID ids[RTC_MAX_EVENT_COUNT + 1];

    /* REGISTER NULL */
    if(register_rtc_event(NULL, 1, &ids[0]) != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_RTC 0\n");
        kernel_panic();
    }

    /* UNREGISTER */
    if(unregister_rtc_event(ids[0]) != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_RTC 1\n");
        kernel_panic();
    }

    /* REGISTER */
    if(register_rtc_event(handler, 1, &ids[0]) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 2\n");
        kernel_panic();
    }

    /* UNREGISTER */
    if(unregister_rtc_event(ids[0]) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 3\n");
        kernel_panic();
    }

    /* UNREGISTER TWICE */
    if(unregister_rtc_event(ids[0]) != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_RTC 4\n");
        kernel_panic();
    }

    /* UNREGISTER UNKNOW ID */
    if(unregister_rtc_event(RTC_MAX_EVENT_COUNT + 1) != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_RTC 5\n");
        kernel_panic();
    }

    /* REGISTER MORE THAN POSSIBLE */
    counter = 0;
    for(i = 0; i < RTC_MAX_EVENT_COUNT; ++i)
    {
        if(register_rtc_event(handler, 1, &ids[i]) != OS_NO_ERR)
        {
            kernel_error("TEST_RTC 6\n");
            kernel_panic();
        }
    }

    /* Wait for event, the test will never pass if not event is detected */
    while(counter == 0);

    for(i = 0; i < RTC_MAX_EVENT_COUNT; ++i)
    {
        if(register_rtc_event(handler, 1, &ids[RTC_MAX_EVENT_COUNT]) !=
           OS_ERR_NO_MORE_FREE_EVENT)
        {
            kernel_error("TEST_RTC 7\n");
            kernel_panic();
        }
    }

    /* UNREGISTER ALL */
    for(i = 0; i < RTC_MAX_EVENT_COUNT; ++i)
    {
        if(unregister_rtc_event(ids[i]) != OS_NO_ERR)
        {
            kernel_error("TEST_RTC 8\n");
            kernel_panic();
        }
    }

    if(counter == 0)
    {
        kernel_error("TEST_RTC 9\n");
        kernel_panic();
    }

    kernel_debug("RTC tests passed\n");

    disable_interrupt();
}
