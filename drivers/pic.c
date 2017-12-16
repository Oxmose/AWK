/*******************************************************************************
 *
 * File: pic.h
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

#include "../cpu/cpu.h" /* outb */
#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E, NULL */
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

	kernel_success("PIC Initialized\n", 16);
}