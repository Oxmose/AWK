/*******************************************************************************
 *
 * File: debug.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/12/2017
 *
 * Version: 1.0
 *
 * Debug configuration for the kernel
 ******************************************************************************/

#ifndef __DEBUG_H_
#define __DEBUG_H_

#define KERNEL_DEBUG

#ifdef KERNEL_DEBUG

#include "core/kernel_output.h" /* kernel_serial_debug */
//#define DEBUG_MAILBOX
//#define DEBUG_QUEUE
//#define DEBUG_ACPI
//#define DEBUG_DRIVER_MANAGER
//#define DEBUG_INTERRUPT
//#define DEBUG_KERNEL_QUEUE
//#define DEBUG_SCHED
//#define DEBUG_ATA
#define DEBUG_LAPIC
#define DEBUG_PIC
#define DEBUG_IOAPIC
#define DEBUG_PIT
//#define DEBUG_MOUSE

#endif /* DEBUG */

#endif /* __DEBUG_H_ */
