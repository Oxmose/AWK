/*******************************************************************************
 *
 * File: bios_call.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 02/01/2018
 *
 * Version: 1.0
 *
 * BIOS interrupt wrapper
 ******************************************************************************/

#ifndef __BIOS_CALL_H_
#define __BIOS_CALL_H_

#include "../lib/stdint.h" /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef volatile struct bios_int_regs
{
    volatile uint16_t di;
    volatile uint16_t si;
    volatile uint16_t bp;
    volatile uint16_t sp;
    volatile uint16_t bx;
    volatile uint16_t dx;
    volatile uint16_t cx;
    volatile uint16_t ax;
    volatile uint16_t gs;
    volatile uint16_t fs;
    volatile uint16_t es;
    volatile uint16_t ds;
    volatile uint16_t eflags;
} __attribute__((__packed__)) bios_int_regs_t;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Issue a bios interrupt.
 *
 * @param intnum The interrupt to raise.
 * @param regs The array containing the registers values for the call.
 */
void bios_int(uint8_t intnum, bios_int_regs_t* regs);

#endif /* __BIOS_CALL_H_  */
