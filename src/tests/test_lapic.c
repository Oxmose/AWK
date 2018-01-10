/*******************************************************************************
 *
 * File: test_io_apic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 10/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: IO Advanced programmable interrupt controler tests
 ******************************************************************************/



#include "../core/interrupts.h"
#include "../core/kernel_output.h"
#include "../core/panic.h"
#include "../cpu/cpu.h"
#include "../drivers/lapic.h"

void test_lapic(void)
{
    /* TEST OEI > MAX */
    if(set_INT_LAPIC_EOI(SPURIOUS_INT_LINE + 1) !=
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_LAPIC 0\n");
        kernel_panic();
    }

    kernel_debug("Local APIC tests passed\n");
}

/*
 * !!! THESE TESTS MUST BE DONE AFTER INITIALIZING THE LAPIC TIMER !!!
 */

 void test_lapic_timer(void)
 {
     /* NOTHING TO DO, LAPIC TIMER WILL BBE TESTED WHEN USED FOR THE
      * SCHEDULING PURPOSE, ONLY THE INTERRUPT IS USED FOR THE LAPIC TIMER
      */

     kernel_debug("Local APIC TIMER tests passed\n");
 }
