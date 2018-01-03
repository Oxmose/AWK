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
#include "../lib/stddef.h" /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct bios_int_regs
{
    uint16_t di;
    uint16_t si;
    uint16_t bp;
    uint16_t sp;
    uint16_t bx;
    uint16_t dx;
    uint16_t cx;
    uint16_t ax;
    uint16_t gs;
    uint16_t fs;
    uint16_t es;
    uint16_t ds;
    uint16_t eflags;
} __attribute__((__packed__)) bios_int_regs_t;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Issue a bios interrupt.
 *
 * @param intnum The interrupt to raise.
 * @param regs The array containing the registers values for the call.
 */
void bios_int(const uint8_t intnum, bios_int_regs_t *regs);

#endif /* __BIOS_CALL_H_  */