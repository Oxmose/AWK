/*******************************************************************************
 *
 * File: test_bios_call.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 10/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Bios interrupts call tests
 ******************************************************************************/

#include "../../core/interrupts.h"
#include "../../core/kernel_output.h"
#include "../../core/panic.h"
#include "../../lib/string.h"
#include "../../bios/bios_call.h"
#include "../../drivers/graphic.h"

void test_bios_call(void)
{
    uint32_t i;
    bios_int_regs_t regs;
    char* str = "BIOS Real Mode Calls tests passed";
    kernel_debug("");

    cursor_t cursor;
    save_cursor(&cursor);

    /* Define cursor position */
    regs.ax = 0x0200;
    regs.bx = 0x0000;
    regs.dx = ((cursor.x & 0xFF)) | ((cursor.y & 0xFF)) << 8;

    bios_int(0x10, &regs);

    /* Write srtring with bios */
    for(i = 0; i < strlen(str); ++i)
    {
        regs.ax = 0x0E00;
        regs.ax |= str[i] & 0x00FF;
        regs.bx = 0x0000;
        regs.cx = 0x0001;

        bios_int(0x10, &regs);
    }

    kernel_printf("\n");
}
