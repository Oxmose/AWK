/*******************************************************************************
 *
 * File: bios_call.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 02/01/2018
 *
 * Version: 1.0
 *
 * BIOS interrupt wrapper
 ******************************************************************************/

#include "../lib/stdint.h"      /* Generic int types */
#include "../core/interrupts.h" /* enable_interrupt, disable_interrupt */

/* Header file */
#include "bios_call.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Assemly function */
extern void _bios_int(uint8_t intnum, bios_int_regs_t* regs);

void bios_int(const uint8_t intnum, bios_int_regs_t* regs)
{
	(void)intnum;
	(void)regs;
	disable_interrupt();
	_bios_int(intnum, regs);
	enable_interrupt();
}