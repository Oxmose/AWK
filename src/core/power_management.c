/*******************************************************************************
 *
 * File: power_management.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * Kernel power management function such as reboot and shutdown routines.
 ******************************************************************************/

#include "../cpu/cpu.h"     /* outb, inb, hlt */
#include "../lib/stddef.h"  /* generic int types */

/* Header file */
#include "power_management.h"

void kernel_reboot(void)
{
	uint8_t out = 0x02;
	uint8_t zero = 0;
	/* Use APIC method */


	/* If didnt work use 8042 reset */
	while ((out & 0x02) != 0)
	{
		out = inb(0x64);
	}
	outb(0x64, 0xFE);

    /* Triple fault */
    __asm__ __volatile__("lidt %0" :: "m" (zero), "m" (zero));
    __asm__ __volatile__("int $0x30");
    hlt();
}
