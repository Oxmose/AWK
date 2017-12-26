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

#include "../drivers/io_apic.h"  /* init_io_apic */
#include "../drivers/pic.h"      /* init_pic */
#include "../drivers/pit.h"      /* init_pit */
#include "../drivers/rtc.h"      /* init_rtc */
#include "../drivers/mouse.h"    /* init_mouse */
#include "../drivers/keyboard.h" /* init_keyboard */
#include "../cpu/cpu.h"          /* get_cpu_info */
#include "../cpu/lapic.h"        /* init_lapic */
#include "../lib/stdio.h"        /* printf */
#include "kernel_output.h"       /* kernel_succes, kernel_error, kernell_info */
#include "scheduler.h"           /* init_scheduler */
#include "panic.h"				 /* kernel_panic */
#include "acpi.h"                /* init_acpi */
#include "driver_manager.h"      /* load_drivers, register_driver, 
                                  * init_driver_manager*/

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

	/* Init ACPI */
	err = init_acpi();
	if(err == OS_NO_ERR)
	{
		kernel_success("ACPI Initialized\n");
	}
	else
	{
		kernel_error("ACPI Initialization error [%d]\n", err);
		//kernel_panic();
	}

	/* Init kernel interrupt handlers */
	err = init_kernel_interrupt();
	if(err == OS_NO_ERR)
	{
		kernel_success("KIH Initialized\n");
	}
	else
	{
		kernel_error("KIH Initialization error [%d]\n", err);
		kernel_panic();
	}

	/* Init driver manager */
	err = init_driver_manager();
	if(err == OS_NO_ERR)
	{
		kernel_success("Driver Manager Initialized\n");
	}
	else
	{
		kernel_error("Driver Manager  Initialization error [%d]\n", err);
		kernel_panic();
	}

	/* Register PIC */
	err = register_driver(init_pic, "PIC");
	if(err != OS_NO_ERR)
	{
		kernel_error("PIC driver registration error [%d]\n", err);
		kernel_panic();
	}

	if(acpi_get_lapic_available() == 1)
	{
		/* Init Local APIC */
		err = register_driver(init_lapic, "LAPIC");
		if(err != OS_NO_ERR)
		{
			kernel_error("LAPIC driver registration error [%d]\n", err);
			kernel_panic();
		}
	}

	if(acpi_get_io_apic_available() == 1)
	{
		kernel_info("IO-APIC present, PIC will be initialized but not used\n");

		/* Register IO APIC */
		err = register_driver(init_io_apic, "IO-APIC");
		if(err != OS_NO_ERR)
		{
			kernel_error("IO-APIC registration error [%d]\n", err);
			kernel_panic();
		}
	}

	/* Register PIT */
	err = register_driver(init_pit, "PIT");
	if(err != OS_NO_ERR)
	{
		kernel_error("PIT driver registration error [%d]\n", err);
		kernel_panic();
	}

	/* Register RTC */
	err = register_driver(init_rtc, "RTC");
	if(err != OS_NO_ERR)
	{
		kernel_error("RTC driver registration error [%d]\n", err);
		kernel_panic();
	}

	/* Register mouse */
	err = register_driver(init_mouse, "MOUSE");
	if(err != OS_NO_ERR)
	{
		kernel_error("MOUSE driver registration error [%d]\n", err);
		kernel_panic();
	}

	/* Register keyboard */
	err = register_driver(init_keyboard, "KEYBOARD");
	if(err != OS_NO_ERR)
	{
		kernel_error("KEYBOARD driver registration error [%d]\n", err);
		kernel_panic();
	}

	/* Load drivers */
	err = load_drivers();
	if(err != OS_NO_ERR)
	{
		kernel_error("Drivers loading failed [%d]\n", err);
		kernel_panic();
	}

	/* Init scheduler, should never come back */
	err = init_scheduler();

	kernel_error("SCHED Initialization error [%d]\n", err);
	kernel_panic();
}