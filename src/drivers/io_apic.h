/*******************************************************************************
 *
 * File: io_apic.h
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

#ifndef __IO_APIC_H_
#define __IO_APIC_H_

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E, NULL */
#include "../core/interrupts.h" /* MIN_INTERRUPT_LINE */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define IO_APIC_BASE_INTERRUPT_LINE MIN_INTERRUPT_LINE

#define IO_APIC_MIN_IRQ_LINE 0
#define IO_APIC_MAX_IRQ_LINE 15

#define IOREGSEL                        0x00
#define IOWIN                           0x10

#define IOAPICID                        0x00
#define IOAPICVER                       0x01
#define IOAPICARB                       0x02
#define IOREDTBL                        0x10

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init the IO-APIC by remapping the IRQ interrupts.
 * Disables all IRQ.
 *
 * @return The state or error code.
 */
OS_RETURN_E init_io_apic(void);

/* Set an entry for the IO-APIC vector.
 *
 * @param irq_number The irq number to enable/disable.
 * @param enabled Must be set to 1 to enable the IRQ or 0 to disable the IRQ.
 * @return The state or error code.
 */
OS_RETURN_E set_IRQ_IO_APIC_mask(const uint32_t irq_number,
                                 const uint8_t enabled);

#endif /* __IO_APIC_H_ */