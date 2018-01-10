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
 * Init the rest of the kernel after GDT and IDT
 * AT THIS POINT INTERRUPT SHOULD BE DISABLED
 ******************************************************************************/

#include "../drivers/lapic.h"    /* init_lapic */
#include "../drivers/io_apic.h"  /* init_ioapic */
#include "../drivers/keyboard.h" /* init_keyboard */
#include "../drivers/mouse.h"    /* init_mouse */
#include "../drivers/rtc.h"      /* init_rtc */
#include "../drivers/pit.h"      /* init_pit */
#include "../drivers/pic.h"      /* init_pic */
#include "../drivers/acpi.h"     /* init_acpi */
#include "../cpu/cpu.h"          /* get_cpu_info */
#include "kernel_output.h"       /* kernel_success, kernel_error, kernel_info */
#include "interrupts.h"          /* init_kernel_interrupt */
#include "panic.h"               /* kernel_panic */

#include "../tests/tests.h"      /* test_bank */

#include "../debug.h"            /* DEBUG */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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
                    kernel_printf("%c", (char)((regs[1] >> (j * 8)) & 0xFF));
                }
                for(int8_t j = 0; j < 4; ++j)
                {

                    kernel_printf("%c", (char)((regs[3] >> (j * 8)) & 0xFF));
                }
                for(int8_t j = 0; j < 4; ++j)
                {

                    kernel_printf("%c", (char)((regs[2] >> (j * 8)) & 0xFF));
                }
                kernel_printf("\n");

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

#ifdef TESTS
    test_sw_interupts();
#endif

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

#ifdef TESTS
    test_pic();
#endif

    /* Init IOAPIC */
    if(acpi_get_io_apic_available() == 1)
    {
        kernel_info("IO-APIC detected, PIC will be desactivated.\n");
        err = init_io_apic();
        if(err == OS_NO_ERR)
        {
            kernel_success("IO-APIC Initialized\n");
        }
        else
        {
            kernel_error("IO-APIC Initialization error [%d]\n", err);
            kernel_panic();
        }
    #ifdef TESTS
        test_io_apic();
    #endif
    }

    /* Init LAPIC */
    if(acpi_get_lapic_available() == 1)
    {
        err = init_lapic();
        if(err == OS_NO_ERR)
        {
            kernel_success("Local APIC Initialized\n");
        }
        else
        {
            kernel_error("Local APIC Initialization error [%d]\n", err);
            kernel_panic();
        }
    #ifdef TESTS
        test_lapic();
    #endif
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
#ifdef TESTS
    test_pit();
#endif

    /* Init LAPIC TIMER */
    if(acpi_get_lapic_available() == 1)
    {
        err = init_lapic_timer();
        if(err == OS_NO_ERR)
        {
            kernel_success("Local APIC TIMER Initialized\n");
        }
        else
        {
            kernel_error("Local APIC TIMER Initialization error [%d]\n", err);
            kernel_panic();
        }
    #ifdef TESTS
        test_lapic_timer();
    #endif
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
#ifdef TESTS
    test_rtc();
#endif

    /* Init MOUSE */
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
#ifdef TESTS
    test_keyboard();
#endif


    /* Init MOUSE */
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
#ifdef TESTS
    test_mouse();
#endif

    enable_interrupt();

    while(1);
}
