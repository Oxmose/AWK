/*******************************************************************************
 *
 * File: test_mouse.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Mouse
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
#include "../drivers/mouse.h"

static volatile uint32_t counter;

static void handler(void)
{
    ++counter;
    return;
}

void test_mouse(void)
{
    int32_t i;

    OS_EVENT_ID ids[MOUSE_MAX_EVENT_COUNT + 1];

    /* REGISTER NULL */
    if(register_mouse_event(NULL, &ids[0]) != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_MOUSE 0\n");
        kernel_panic();
    }

    /* UNREGISTER */
    if(unregister_mouse_event(ids[0]) != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_MOUSE 1\n");
        kernel_panic();
    }

    /* REGISTER */
    if(register_mouse_event(handler, &ids[0]) != OS_NO_ERR)
    {
        kernel_error("TEST_MOUSE 2\n");
        kernel_panic();
    }

    /* UNREGISTER */
    if(unregister_mouse_event(ids[0]) != OS_NO_ERR)
    {
        kernel_error("TEST_MOUSE 3\n");
        kernel_panic();
    }

    /* UNREGISTER TWICE */
    if(unregister_mouse_event(ids[0]) != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_MOUSE 4\n");
        kernel_panic();
    }

    /* UNREGISTER UNKNOW ID */
    if(unregister_mouse_event(MOUSE_MAX_EVENT_COUNT + 1) != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_MOUSE 5\n");
        kernel_panic();
    }

    /* REGISTER MORE THAN POSSIBLE */
    for(i = 0; i < MOUSE_MAX_EVENT_COUNT; ++i)
    {
        if(register_mouse_event(handler, &ids[i]) != OS_NO_ERR)
        {
            kernel_error("TEST_MOUSE 6\n");
            kernel_panic();
        }
    }

    for(i = 0; i < MOUSE_MAX_EVENT_COUNT; ++i)
    {
        if(register_mouse_event(handler, &ids[MOUSE_MAX_EVENT_COUNT]) !=
           OS_ERR_NO_MORE_FREE_EVENT)
        {
            kernel_error("TEST_MOUSE 7\n");
            kernel_panic();
        }
    }

    /* UNREGISTER ALL */
    for(i = 0; i < MOUSE_MAX_EVENT_COUNT; ++i)
    {
        if(unregister_mouse_event(ids[i]) != OS_NO_ERR)
        {
            kernel_error("TEST_MOUSE 8\n");
            kernel_panic();
        }
    }


    kernel_debug("MOUSE tests passed\n");

}
