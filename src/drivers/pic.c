/*******************************************************************************
 *
 * File: pic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * PIC (programmable interrupt controler) driver.
 * Allows to remmap the PIC IRQ, set the IRQs mask, manage EoI.
 *
 ******************************************************************************/

#include "../cpu/cpu.h"            /* outb */
#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/stddef.h"         /* OS_RETURN_E, NULL */
#include "../core/kernel_output.h" /* kernel_success */

/* Header file */
#include "pic.h"

void setup_pic()
{
	/* Initialize the master. */
	outb(PIC_ICW1_ICW4 | PIC_ICW1_INIT, PIC_MASTER_COMM_PORT);
	outb(PIC0_BASE_INTERRUPT_LINE, PIC_MASTER_DATA_PORT);
	outb(0x4,  PIC_MASTER_DATA_PORT);
	outb(0x1,  PIC_MASTER_DATA_PORT);
	
	/* Initialize the slave. */
	outb(PIC_ICW1_ICW4 | PIC_ICW1_INIT, PIC_SLAVE_COMM_PORT);
	outb(PIC1_BASE_INTERRUPT_LINE, PIC_SLAVE_DATA_PORT);
	outb(0x2,  PIC_SLAVE_DATA_PORT);
	outb(0x1,  PIC_SLAVE_DATA_PORT);
	
	/* Set EOI for both PICs. */
	outb(0x20, PIC_MASTER_COMM_PORT);
	outb(0x20, PIC_SLAVE_COMM_PORT);
	
	/* Disable all IRQs */
	outb(0xFF, PIC_MASTER_DATA_PORT);
	outb(0xFF, PIC_SLAVE_DATA_PORT);

	kernel_success("PIC Initialized\n");
}

OS_RETURN_E set_IRQ_mask(const uint32_t IRQ_number, const uint8_t enabled)
{
    if(IRQ_number > MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    /* Manage master PIC */
    if(IRQ_number < 8)
    {
        /* Retrieve initial mask */
        uint8_t init_mask = inb(PIC_MASTER_DATA_PORT); 

        /* Set new mask value */
        if(!enabled)
        {
            init_mask |= 1 << IRQ_number;
        }
        else
        {
            init_mask &= ~(1 << IRQ_number);
        }

        /* Set new mask */
        outb(init_mask, PIC_MASTER_DATA_PORT);
    }

    /* Manage slave PIC. WARNING, cascading has to be enabled */
    if(IRQ_number > 7)
    {
        /* Set new IRQ number */
         uint32_t cascading_number = IRQ_number - 8;

        /* Retrieve initial mask */
        uint8_t init_mask = inb(PIC_SLAVE_DATA_PORT); 

        /* Set new mask value */
        if(!enabled)
        {
            init_mask |= 1 << cascading_number;
        }
        else
        {
            init_mask &= ~(1 << cascading_number);
        }

        /* Set new mask */
        outb(init_mask, PIC_SLAVE_DATA_PORT);
    }

    return OS_NO_ERR;
}

OS_RETURN_E set_IRQ_EOI(const uint32_t IRQ_number)
{ 
    if(IRQ_number > MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }
    
    /* End of interrupt signal */
    if(IRQ_number > 7)
    {
        outb(PIC_EOI, PIC_SLAVE_COMM_PORT);
    }
    outb(PIC_EOI, PIC_MASTER_COMM_PORT);

    return OS_NO_ERR;
}