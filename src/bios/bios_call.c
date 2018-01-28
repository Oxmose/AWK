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
#include "../memory/paging.h"   /* disable_paging enable_paging */
#include "../core/interrupts.h" /* enable_local_interrupt,
                                   disable_local_interrupt */

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

void bios_int(uint8_t intnum, bios_int_regs_t* regs)
{
	disable_local_interrupt();
	disable_paging();

	_bios_int(intnum, regs);

	enable_paging();
	enable_local_interrupt();
}
