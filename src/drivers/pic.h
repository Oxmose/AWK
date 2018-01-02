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
 ******************************************************************************/

#ifndef __PIC_H_
#define __PIC_H_

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E, NULL */
#include "../core/interrupts.h" /* INT_IRQ_OFFSET */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define PIC_MASTER_COMM_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_COMM_PORT  0xa0
#define PIC_SLAVE_DATA_PORT  0xa1
#define PIC_EOI              0x20

#define PIC_ICW1_ICW4        0x01
#define PIC_ICW1_SINGLE      0x02
#define PIC_ICW1_INTERVAL4   0x04
#define PIC_ICW1_LEVEL       0x08
#define PIC_ICW1_INIT        0x10

#define PIC0_BASE_INTERRUPT_LINE INT_IRQ_OFFSET
#define PIC1_BASE_INTERRUPT_LINE (INT_IRQ_OFFSET + 8)


#define PIC_MIN_IRQ_LINE 0
#define PIC_MAX_IRQ_LINE 15

#define PIC_CASCADING_IRQ 2

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init the PIC by remapping the IRQ interrupts.
 * Disable al IRQ by reseting the IRQs mask.
 * @return The state or error code.
 */
OS_RETURN_E init_pic(void);

/* Set the IRQ mask for the IRQ number given as parameter.
 * @param irq_number The irq number to enable/disable.
 * @param enabled Must be set to 1 to enable the IRQ or 0 to disable the IRQ.
 * @return The state or error code.
 */
OS_RETURN_E set_IRQ_PIC_mask(const uint32_t irq_number, const uint8_t enabled);

/* Acknomledge the IRQ.
 * @param irq_number The irq number to acknowledge.
 * @return The state or error code.
 */
OS_RETURN_E set_IRQ_PIC_EOI(const uint32_t irq_number);

#endif /* __PIC_H_ */