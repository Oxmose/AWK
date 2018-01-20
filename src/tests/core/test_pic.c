/*******************************************************************************
 *
 * File: test_pic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Programmable interrupt controler tests
 ******************************************************************************/

/*
 * !!! THESE TESTS MUST BE DONE BEFORE INITIALIZING ANY INTERRUPT HANDLER
 *     BETWEEN MIN_INTERRUPT_LINE AND MAX_INTERRUPT_LINE !!!
 * !!! THESE TESTS MUST BE DONE AFTER INITIALIZING THE PIC AND BEFORE THE
 *     IOAPIC!!!
 */

#include "../../core/interrupts.h"
#include "../../core/kernel_output.h"
#include "../../core/panic.h"
#include "../../cpu/cpu.h"
#include "../../drivers/pic.h"

void test_pic(void)
{
    uint8_t  pic0_mask;
    uint8_t  pic1_mask;
    uint8_t  pic0_mask_save;
    uint8_t  pic1_mask_save;
    uint32_t i;

    disable_local_interrupt();

    /* TEST MASK > MAX */
    if(set_IRQ_PIC_mask(PIC_MAX_IRQ_LINE + 1, 0) != OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_PIC 0\n");
        kernel_panic();
    }

    /* TEST EOI > MAX */
    if(set_IRQ_PIC_EOI(PIC_MAX_IRQ_LINE + 1) != OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_PIC 1\n");
        kernel_panic();
    }

    /* Save current PIC mask */
    pic0_mask_save = inb(PIC_MASTER_DATA_PORT);
    pic1_mask_save = inb(PIC_SLAVE_DATA_PORT);

    /* TEST MASK SET */
    for(i = 0; i <= PIC_MAX_IRQ_LINE; ++i)
    {
        if(set_IRQ_PIC_mask(i, 1) != OS_NO_ERR)
        {
            kernel_error("TEST_PIC 2\n");
            kernel_panic();
        }
    }

    pic0_mask = inb(PIC_MASTER_DATA_PORT);
    pic1_mask = inb(PIC_SLAVE_DATA_PORT);

    if(pic0_mask != 0 || pic1_mask != 0)
    {
        kernel_error("TEST_PIC 3\n");
        kernel_panic();
    }

    /* TEST MASK CLEAR */
    for(i = 0; i <= PIC_MAX_IRQ_LINE; ++i)
    {
        if(set_IRQ_PIC_mask(i, 0) != OS_NO_ERR)
        {
            kernel_error("TEST_PIC 4\n");
            kernel_panic();
        }
    }

    pic0_mask = inb(PIC_MASTER_DATA_PORT);
    pic1_mask = inb(PIC_SLAVE_DATA_PORT);

    if(pic0_mask != 0xFF || pic1_mask != 0xFF)
    {
        kernel_error("TEST_PIC 5\n");
        kernel_panic();
    }

    /* Restore mask */
    outb(pic0_mask_save, PIC_MASTER_DATA_PORT);
    outb(pic1_mask_save, PIC_SLAVE_DATA_PORT);

    kernel_debug("PIC tests passed\n");

    enable_local_interrupt();
}
