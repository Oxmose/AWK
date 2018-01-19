/*******************************************************************************
 *
 * File: test_pit.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Programmable Interval Timer tests
 ******************************************************************************/

/*
 * !!! THESE TESTS MUST BE DONE BEFORE INITIALIZING ANY INTERRUPT HANDLER
 *     BETWEEN MIN_INTERRUPT_LINE AND MAX_INTERRUPT_LINE !!!
 * !!! THESE TESTS MUST BE DONE AFTER INITIALIZING THE PIC AND OR THE IOAPIC!!!
 */

#include "../../core/interrupts.h"
#include "../../core/kernel_output.h"
#include "../../core/panic.h"
#include "../../cpu/cpu.h"
#include "../../drivers/pit.h"

static volatile uint32_t counter;

static void pit_handler(cpu_state_t* cpu, uint32_t id, stack_state_t* stack)
{
    (void)cpu;
    (void)id;
    (void)stack;

    counter++;
    set_IRQ_EOI(PIT_IRQ_LINE);
}

void test_pit(void)
{
    volatile uint32_t i;
    volatile uint32_t cnt_val;

    enable_interrupt();

    /* REGISTER NULL */
    if(set_pit_handler(NULL) != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_PIT 0\n");
        kernel_panic();
    }

    /* REGISTER OUR HANNDLER */
    if(set_pit_handler(pit_handler) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 1\n");
        kernel_panic();
    }

    /* REMOVE */
    if(remove_pit_handler() != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 2\n");
        kernel_panic();
    }

    /* REGISTER OUR HANNDLER */
    if(set_pit_handler(pit_handler) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 3\n");
        kernel_panic();
    }

    /* CHECK ENABLE / DISABLE*/
    counter = 0;
    cnt_val = 0;

    enable_pit();

    for(i = 0; i < 10000000; ++i);
    disable_pit();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_PIT 4\n");
        kernel_panic();
    }



    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 5\n");
        kernel_panic();
    }

    counter = 0;
    enable_pit();

    for(i = 0; i < 10000000; ++i);

    disable_pit();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_PIT 6\n");
        kernel_panic();
    }



    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 7\n");
        kernel_panic();
    }


    if(set_pit_freq(PIT_INIT_FREQ) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 8\n");
        kernel_panic();
    }

    if(set_pit_freq(PIT_MIN_FREQ - 1) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_PIT 9\n");
        kernel_panic();
    }

    if(set_pit_freq(PIT_MAX_FREQ + 1) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_PIT 10\n");
        kernel_panic();
    }

    /* Check if the PIT did not erenabled itself between */
    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 11\n");
        kernel_panic();
    }

    /* REMOVE */
    if(remove_pit_handler() != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 12\n");
        kernel_panic();
    }

    kernel_debug("PIT tests passed\n");

    enable_pit();
    disable_interrupt();
}
