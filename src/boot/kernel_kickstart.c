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
 * Init the rest of the kernel after GDT and IDT and HEAP
 * AT THIS POINT INTERRUPT SHOULD BE DISABLED
 ******************************************************************************/

#include "../memory/paging.h"       /* mem_range_t */
#include "../lib/string.h"          /* memcpy */
#include "../drivers/ata.h"         /* init_ata */
#include "../drivers/serial.h"      /* init_serial */
#include "../drivers/vesa.h"        /* init_vesa */
#include "../drivers/lapic.h"       /* init_lapic */
#include "../drivers/io_apic.h"     /* init_ioapic */
#include "../drivers/keyboard.h"    /* init_keyboard */
#include "../drivers/mouse.h"       /* init_mouse */
#include "../drivers/rtc.h"         /* init_rtc */
#include "../drivers/pit.h"         /* init_pit */
#include "../drivers/pic.h"         /* init_pic */
#include "../drivers/acpi.h"        /* init_acpi */
#include "../cpu/cpu.h"             /* get_cpu_info */
#include "../core/scheduler.h"      /* init_scheduler */
#include "../core/interrupts.h"     /* init_kernel_interrupt */
#include "../core/panic.h"          /* kernel_panic */
#include "../core/kernel_output.h"  /* kernel_success, kernel_error,
                                       kernel_info */
#include "multiboot.h"           /* multiboot_header_t */

#include "../tests/core/tests.h"      /* test_bank */

#include "../debug.h"            /* DEBUG */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

extern multiboot_info_t* multiboot_data_ptr;
extern uint32_t          multiboot_data_size;
extern mem_range_t       multiboot_data[];

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init each basic drivers for the kernel, then init the scheduler and
 * start the system.
 */
void kernel_kickstart(void)
{
    OS_RETURN_E err;
    cpu_info_t cpu_info;

    uint32_t regs[4]; //eax, ebx, ecx, edx;
    uint32_t ret;
    uint32_t i;
    multiboot_memory_map_t* mmap;


    /* Copy multiboot data in upper memory */
    mmap = (multiboot_memory_map_t*)multiboot_data_ptr->mmap_addr;
    i = 0;
    while((uint32_t)mmap < multiboot_data_ptr->mmap_addr + multiboot_data_ptr->mmap_length)
    {
        multiboot_data[i].base  = (uint32_t)mmap->addr;
        multiboot_data[i].limit = (uint32_t)mmap->addr + (uint32_t)mmap->len;
        multiboot_data[i].type  = mmap->type;
        ++i;
        mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
    multiboot_data_size = i - 1;

    for(i = 0; i < multiboot_data_size; ++i)
    {
        kernel_info("Base 0x%08x, Limit 0x%08x, Length %uBytes, Type %d\n", multiboot_data[i].base, multiboot_data[i].limit, multiboot_data[i].base + multiboot_data[i].limit, multiboot_data[i].type);
    }

    /* Init paging */
    //err = init_paging();
    //if(err == OS_NO_ERR)
    //{
    //    kernel_success("Paging Initialized\n");
    //}
    //else
    //{
    //    kernel_error("Paging Initialization error [%d]\n", err);
    //}

    /* Get CPUID info */
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

#ifdef TESTS
    test_bios_call();
    test_klist();
#endif

    /* Init VESA */
    err = init_vesa();
    if(err == OS_NO_ERR)
    {
        kernel_success("VESA Initialized\n");
    }
    else
    {
        kernel_error("VESA Initialization error [%d]\n", err);
    }
    err = text_vga_to_vesa();
    if(err != OS_NO_ERR)
    {
        kernel_error("VESA swtich error [%d]\n", err);
    }

    /* Init SERIAL */
    err = init_serial();
    if(err == OS_NO_ERR)
    {
        kernel_success("SERIAL Initialized\n");
    }
    else
    {
        kernel_error("SERIAL Initialization error [%d]\n", err);
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
        kernel_info("IO-APIC detected, PIC will be desactivated\n");
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

    /* Init ATA PIO drivers */
    err = init_ata();
    if(err == OS_NO_ERR)
    {
        kernel_success("ATA-PIO Initialized\n");
    }
    else
    {
        kernel_error("ATA-PIO Initialization error [%d]\n", err);
        kernel_panic();
    }
#ifdef TESTS
    //test_ata();
#endif

    /* Init Scheduler */
    err = init_scheduler();

    kernel_error("Scheduelr returned [%d]\n", err);
    kernel_panic();

    while(1)
    {
        hlt();
    }
}
