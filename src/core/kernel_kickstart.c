/*******************************************************************************
 *
 * File: kernel_kickstart.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/12/2017
 *
 * Version: 1.0
 *
 * Init the rest of the kernel after GDT, IDT and PIC have been initialized.
 * AT THIS POINT INTERRUPT SHOULD BE DISABLED
 ******************************************************************************/

#include "../drivers/pic.h"      /* init_pic */
#include "../drivers/pit.h"      /* init_pit */
#include "../drivers/rtc.h"      /* init_rtc */
#include "../drivers/mouse.h"    /* init_mouse */
#include "../drivers/keyboard.h" /* init_keyboard */
#include "../cpu/cpu.h"          /* get_cpu_info */
#include "../lib/stdio.h"        /* printf */
#include "kernel_output.h"       /* kernel_succes, kernel_error, kernell_info */
#include "scheduler.h"           /* init_scheduler */
#include "panic.h"				 /* kernel_panic */

void kernel_kickstart(void)
{
	OS_RETURN_E err;
	cpu_info_t cpu_info;

	uint32_t regs[4]; //eax, ebx, ecx, edx;
	uint32_t ret;

	err = get_cpu_info(&cpu_info);
	if(err == OS_NO_ERR)
	{
		if(cpuid_capable() == 1)
		{
			ret = cpuid(0, regs);
			if(ret != 0)
			{
				kernel_info("CPUID Available | Vendor ");

				for(int8_t j = 0; j < 4; ++j)
				{
				
					printf("%c", (char)((regs[1] >> (j * 8)) & 0xFF));
				}
				for(int8_t j = 0; j < 4; ++j)
				{
				
					printf("%c", (char)((regs[3] >> (j * 8)) & 0xFF));
				}
				for(int8_t j = 0; j < 4; ++j)
				{
				
					printf("%c", (char)((regs[2] >> (j * 8)) & 0xFF));
				}
				printf("\n");
				
			}

		}
	}
	else
	{
		kernel_error("CPU probe error [%d]\n", err);
		kernel_panic();
	}	

	/* Init PIC */
	err = init_pic();
	if(err == OS_NO_ERR)
	{
		kernel_success("PIC Initialized\n");
	}
	else
	{
		kernel_error("PIC Initialization error [%d]\n", err);
		kernel_panic();
	}

	/* Init PIT */
	err = init_pit();
	if(err == OS_NO_ERR)
	{
		kernel_success("PIT Initialized\n");
	}
	else
	{
		kernel_error("PIT Initialization error [%d]\n", err);
		kernel_panic();
	}

	/* Init RTC */
	err = init_rtc();
	if(err == OS_NO_ERR)
	{
		kernel_success("RTC Initialized\n");
	}
	else
	{
		kernel_error("RTC Initialization error [%d]\n", err);
		kernel_panic();
	}

	/* Init mouse */
	err = init_mouse();
	if(err == OS_NO_ERR)
	{
		kernel_success("MOUSE Initialized\n");
	}
	else
	{
		kernel_error("MOUSE Initialization error [%d]\n", err);
		kernel_panic();
	}

	/* Init keyboard */
	err = init_keyboard();
	if(err == OS_NO_ERR)
	{
		kernel_success("KEYBOARD Initialized\n");
	}
	else
	{
		kernel_error("KEYBOARD Initialization error [%d]\n", err);
		kernel_panic();
	}

	/* Init scheduler, should never come back */
	err = init_scheduler();

	kernel_error("SCHED Initialization error [%d]\n", err);
	kernel_panic();
}