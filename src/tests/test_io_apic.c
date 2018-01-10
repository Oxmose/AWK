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

/*
 * !!! THESE TESTS MUST BE DONE BEFORE INITIALIZING ANY INTERRUPT HANDLER
 *     BETWEEN MIN_INTERRUPT_LINE AND MAX_INTERRUPT_LINE !!!
 */

#include "../core/interrupts.h"
#include "../core/kernel_output.h"
#include "../core/panic.h"
#include "../cpu/cpu.h"
#include "../drivers/io_apic.h"

void test_io_apic(void)
{
    /* TEST MASK > MAX */
    if(set_IRQ_IO_APIC_mask(IO_APIC_MAX_IRQ_LINE + 1, 0) !=
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_IOAPIC 0\n");
        kernel_panic();
    }

    /* TEST MASK <= MAX */
    if(set_IRQ_IO_APIC_mask(IO_APIC_MAX_IRQ_LINE , 1) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 1\n");
        kernel_panic();
    }

    if(set_IRQ_IO_APIC_mask(IO_APIC_MAX_IRQ_LINE , 0) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 2\n");
        kernel_panic();
    }


    kernel_debug("IO-APIC tests passed\n");
}
