/*******************************************************************************
 *
 * File: lapic.h
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

#ifndef __LAPIC_H_
#define __LAPIC_H_

#include "../lib/stdint.h"       /* Generic int types */
#include "../lib/stddef.h"       /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
#define LAPIC_ID                        0x0020
#define LAPIC_VER                       0x0030
#define LAPIC_TPR                       0x0080
#define LAPIC_APR                       0x0090
#define LAPIC_PPR                       0x00A0
#define LAPIC_EOI                       0x00B0
#define LAPIC_RRD                       0x00C0
#define LAPIC_LDR                       0x00D0
#define LAPIC_DFR                       0x00E0
#define LAPIC_SVR                       0x00F0
#define LAPIC_ISR                       0x0100
#define LAPIC_TMR                       0x0180
#define LAPIC_IRR                       0x0200
#define LAPIC_ESR                       0x0280
#define LAPIC_ICRLO                     0x0300
#define LAPIC_ICRHI                     0x0310
#define LAPIC_TIMER                     0x0320
#define LAPIC_THERMAL                   0x0330
#define LAPIC_PERF                      0x0340
#define LAPIC_LINT0                     0x0350
#define LAPIC_LINT1                     0x0360
#define LAPIC_ERROR                     0x0370
#define LAPIC_TICR                      0x0380
#define LAPIC_TCCR                      0x0390
#define LAPIC_TDCR                      0x03E0

/* Delivery Mode */
#define ICR_FIXED                       0x00000000
#define ICR_LOWEST                      0x00000100
#define ICR_SMI                         0x00000200
#define ICR_NMI                         0x00000400
#define ICR_INIT                        0x00000500
#define ICR_STARTUP                     0x00000600

/* Destination Mode */
#define ICR_PHYSICAL                    0x00000000
#define ICR_LOGICAL                     0x00000800

/* Delivery Status */
#define ICR_IDLE                        0x00000000
#define ICR_SEND_PENDING                0x00001000

/* Level */
#define ICR_DEASSERT                    0x00000000
#define ICR_ASSERT                      0x00004000

/* Trigger Mode */
#define ICR_EDGE                        0x00000000
#define ICR_LEVEL                       0x00008000

/* Destination Shorthand */
#define ICR_NO_SHORTHAND                0x00000000
#define ICR_SELF                        0x00040000
#define ICR_ALL_INCLUDING_SELF          0x00080000
#define ICR_ALL_EXCLUDING_SELF          0x000C0000

/* Destination Field */
#define ICR_DESTINATION_SHIFT           24

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init CPU Local APIC
 *
 * @return OS_NO_ERR on succes, an error otherwise.
 */
OS_RETURN_E init_lapic(void);

/* Returns the current CPU Local APIC ID.
 *
 * @returns The current CPU Local APIC ID.
 */
uint32_t get_lapic_id(void);

/* Send an INIT IPI to the CPU deisgned by the Local APIC ID given as parameter.
 * The ID is checked before sending the IPI.
 *
 * @param lapic_id The Local APIC ID of the CPU to send the IPI to.
 * @return OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E lapic_send_ipi_init(const uint32_t lapic_id);

/* Send an STARTUP IPI to the CPU deisgned by the Local APIC ID given as parameter.
 * The ID is checked before sending the IPI.
 *
 * @param lapic_id The Local APIC ID of the CPU to send the IPI to.
 * @param vector The startup IPI vector.
 * @return OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id,
                                   const uint32_t vector);

/* Set END OF INTERRUPT for the current CPU Local APIC.
 *
 * @param interrupt_line The intrrupt line for which the EOI should be set.
 * @return OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E set_INT_LAPIC_EOI(const uint32_t interrupt_line);

#endif /* __LAPIC_H_ */
