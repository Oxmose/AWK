/*******************************************************************************
 *
 * File: lapic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/12/2017
 *
 * Version: 1.0
 *
 * Local APIC (Advanced programmable interrupt controler) driver.
 *
 ******************************************************************************/

#include "../lib/stdint.h"       /* Generic int types */
#include "../lib/stddef.h"       /* OS_RETURN_E */
#include "../core/interrupts.h"  /* SPURIOUS_INTERRUPT_LINE */
#include "../core/acpi.h"        /* get_lapic_addr */
#include "cpu.h"                 /* mapped_io_read_32, mapped_io_write_32 */

/* Header file */
#include "lapic.h"

/* Local APIC address */
static uint8_t *lapic_base_addr;

static uint32_t lapic_read(uint32_t reg)
{
    return mapped_io_read_32(lapic_base_addr + reg);
}

static void lapic_write(uint32_t reg, uint32_t data)
{
    mapped_io_write_32(lapic_base_addr + reg, data);
}

OS_RETURN_E init_lapic(void)
{
	/* Get Local APIC base address */
    lapic_base_addr = get_lapic_addr();

	/* Enable all interrupts */
    lapic_write(LAPIC_TPR, 0);

    /* Set logical destination mode */
    lapic_write(LAPIC_DFR, 0xffffffff);
    lapic_write(LAPIC_LDR, 0x01000000);

    /* Spurious Interrupt Vector Register */
    lapic_write(LAPIC_SVR, 0x100 | SPURIOUS_INTERRUPT_LINE);

    return OS_NO_ERR;
}

uint32_t get_lapic_id(void)
{
	return (lapic_read(LAPIC_ID) >> 24);
}

OS_RETURN_E lapic_send_ipi_init(const uint32_t lapic_id)
{
	OS_RETURN_E err;

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        return err;
    }

	lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, ICR_INIT | ICR_PHYSICAL
        | ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    return err;
}

OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id, 
                                   const uint32_t vector)
{
    OS_RETURN_E err;

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        return err;
    }

	lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, vector | ICR_STARTUP
        | ICR_PHYSICAL | ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

	return err;
}

OS_RETURN_E set_INT_LAPIC_EOI(const uint32_t interrupt_line)
{
	if(interrupt_line > SPURIOUS_INTERRUPT_LINE)
	{
		return OS_ERR_NO_SUCH_IRQ_LINE;
	}

	lapic_write(LAPIC_EOI, 0);

	return OS_NO_ERR;
}
