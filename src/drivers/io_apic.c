/*******************************************************************************
 *
 * File: io_apic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/12/2017
 *
 * Version: 1.0
 *
 * IO-APIC (IO advanced programmable interrupt controler) driver.
 *
 ******************************************************************************/

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E, NULL */
#include "../core/interrupts.h" /* INT_IRQ_OFFSET */
#include "../cpu/cpu.h"         /* mapped_io_read_32, mapped_io_write_32 */
#include "acpi.h"               /* acpi_get_io_apic_address */

#include "../debug.h"           /* kernel_serial_debug */

/* Header file */
#include "io_apic.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* IO APIC base address */
static uint8_t* io_apic_base_addr;

/* IRQ redirection count */
static uint32_t max_redirect_count;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Write to the IO APIC controller memory.
 *
 * @param reg The register to write.
 * @param val The value to write to the register.
 */
__inline__ static void io_apic_write(const uint8_t reg, const uint32_t val)
{
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOREGSEL), reg);
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOWIN), val);
}

/* Read into the IO APIC controller memory.
 *
 * @param reg The register to read.
 * @returns The value contained in the register.
 */
__inline__ static uint32_t io_apic_read(const uint8_t reg)
{
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOREGSEL), reg);
    return mapped_io_read_32((uint32_t*)(io_apic_base_addr + IOWIN));
}

OS_RETURN_E init_io_apic(void)
{
    uint32_t    i;
    uint32_t    read_count;
    OS_RETURN_E err;

    /* Get IO APIC base address */
    io_apic_base_addr = acpi_get_io_apic_address();

    /* Maximum entry count */
    read_count = io_apic_read(IOAPICVER);

    max_redirect_count = ((read_count >> 16) & 0xff) + 1;

    /* Disable all interrupts */
    for (i = 0; i < max_redirect_count; ++i)
    {
        err = set_IRQ_IO_APIC_mask(i, 0);
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }

    return OS_NO_ERR;
}

OS_RETURN_E set_IRQ_IO_APIC_mask(const uint32_t irq_number, const uint8_t enabled)
{
    uint32_t entry_lo   = 0;
    uint32_t entry_hi   = 0;
    uint32_t actual_irq = 0;

    if(irq_number >= max_redirect_count || irq_number > IO_APIC_MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    /* Get the remapped value */
    actual_irq = acpi_get_remmaped_irq(irq_number);

    /* Set the interrupt line */
    entry_lo |= irq_number + INT_IRQ_OFFSET;

    /* Set enable mask */
    entry_lo |= (~enabled & 0x1) << 16;

    io_apic_write(IOREDTBL + actual_irq * 2, entry_lo);
    io_apic_write(IOREDTBL + actual_irq * 2 + 1, entry_hi);

    #ifdef DEBUG_IOAPIC
    kernel_serial_debug("IOAPIC mask IRQ %d (%d): %d\n",
                        irq_number, actual_irq, enabled);
    #endif

    return OS_NO_ERR;
}
