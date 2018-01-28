/*******************************************************************************
 *
 * File: acpi.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 22/12/2017
 *
 * Version: 1.0
 *
 * Kernel ACPI management.
 ******************************************************************************/

#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/string.h"         /* memcpy */
#include "../cpu/cpu.h"            /* MAX_CPU_COUNT */
#include "../core/kernel_output.h" /* kernel_error */
#include "../memory/paging.h"      /* kernel_mmap */

#include "../debug.h"              /* DEBUG */

/* Header file */
#include "acpi.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* CPU informations */
static uint32_t cpu_count;
static uint32_t cpu_ids[MAX_CPU_COUNT];

/* CPU LAPIC */
static local_apic_t* cpu_lapic[MAX_CPU_COUNT];

/* IO APIC */
static uint32_t   io_apic_count;
static io_apic_t* io_apic_tables[MAX_IO_APIC_COUNT];

/* ACPI Tables pointers */
static uint8_t              rsdp_parse_success;
static rsdp_descriptor_2_t* rsdp;

static uint8_t            has_xsdt;
static uint8_t            rsdt_parse_success;
static rsdt_descriptor_t* rsdt;
static uint8_t            xsdt_parse_success;
static xsdt_descriptor_t* xsdt;

static uint8_t      fadt_parse_success;
static acpi_fadt_t* fadt;

static uint8_t      facs_parse_success;
static acpi_facs_t* facs;

static uint8_t      dsdt_parse_success;
static acpi_dsdt_t* dsdt;

static uint8_t      madt_parse_success;
static acpi_madt_t* madt;

static uint8_t acpi_init = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Parse the APIC entries of the MADT table.
 * The function will parse each entry and detect two fo the possible entries
 * kind: the LAPIC entries, which also determine the cpu count; the IO-APIC
 * entries will detect the different available IO-APIC od the system.
 *
 * @param madt_ptr The address of the MADT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_apic(acpi_madt_t* madt_ptr)
{
    int32_t        sum;
    uint32_t       i;
    uint8_t*       madt_entry;
    uint8_t*       madt_limit;
    uint8_t        type;
    apic_header_t* header;
    OS_RETURN_E    err;

    if(madt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing MADT at 0x%08x\n", (uint32_t)madt_ptr);
    #endif


    /* Mapping MADP */
    err = kernel_mmap((uint8_t*)madt_ptr,
                      (uint8_t*)madt_ptr,
                      sizeof(acpi_madt_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < madt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)madt_ptr)[i];
    }

    if((sum & 0xFF) != 0)
    {
        kernel_error("MADT Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    if(*((uint32_t*)madt_ptr->header.signature) != ACPI_APIC_SIG)
    {
        kernel_error("MADT Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    madt_entry = (uint8_t*)(madt_ptr + 1);
    madt_limit = ((uint8_t*)madt_ptr) + madt_ptr->header.length;

    cpu_count = 0;
    io_apic_count = 0;
    while (madt_entry < madt_limit)
    {
        /* Get entry header */
        header = (apic_header_t*)madt_entry;
        type = header->type;

        /* Check entry type */
        if(type == APIC_TYPE_LOCAL_APIC)
        {
            #ifdef DEBUG_ACPI
            kernel_serial_debug("Found LAPIC: CPU #%d | ID #%d | FLAGS %x\n",
                                ((local_apic_t*)madt_entry)->acpi_cpu_id,
                                ((local_apic_t*)madt_entry)->apic_id,
                                ((local_apic_t*)madt_entry)->flags);
            #endif

            if(cpu_count < MAX_CPU_COUNT)
            {
                /* Add CPU info to the lapic table */
                cpu_lapic[cpu_count] = (local_apic_t*)madt_entry;
                cpu_ids[cpu_count] = cpu_lapic[cpu_count]->acpi_cpu_id;
                ++cpu_count;
            }
            else
            {
                kernel_info("Exceeded CPU max count, ignoring.\n");
            }
        }
        else if(type == APIC_TYPE_IO_APIC)
        {
            #ifdef DEBUG_ACPI
            kernel_serial_debug("Found IO-APIC ADDR #%08x | ID #%d | GSIB %x\n",
                                ((io_apic_t*)madt_entry)->io_apic_addr,
                                ((io_apic_t*)madt_entry)->apic_id,
                                ((io_apic_t*)madt_entry)->
                                    global_system_interrupt_base);
            #endif

            if(io_apic_count < MAX_IO_APIC_COUNT)
            {
                /* Add IO APIC info to the table */
                io_apic_tables[io_apic_count] = (io_apic_t*)madt_entry;
                ++io_apic_count;
            }
            else
            {
                kernel_info("Exceeded IO APIC max count, ignoring.\n");
            }

        }

        madt_entry += header->length;
    }

    acpi_init = 1;

    return OS_NO_ERR;
}

/* Parse the APIC FACS table.
 * The function will save the FACS table address in for further use.
 *
 * @param facs_ptr The address of the FACS entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_facs(acpi_facs_t* facs_ptr)
{
    OS_RETURN_E err;

    if(facs_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing FACS at 0x%08x\n", (uint32_t)facs_ptr);
    #endif

    /* Mapping FACS */
    err = kernel_mmap((uint8_t*)facs_ptr,
                      (uint8_t*)facs_ptr,
                      sizeof(acpi_facs_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    if(*((uint32_t*)facs_ptr->header.signature) != ACPI_FACS_SIG)
    {
        kernel_error("FACS Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    return OS_NO_ERR;
}

/* Parse the APIC DSDT table.
 * The function will save the DSDT table address in for further use.
 *
 * @param dsdt_ptr The address of the DSDT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_dsdt(acpi_dsdt_t* dsdt_ptr)
{
    int32_t     sum;
    uint32_t    i;
    OS_RETURN_E err;

    if(dsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing DSDT at 0x%08x\n", (uint32_t)dsdt_ptr);
    #endif

    /* Mapping DSDT */
    err = kernel_mmap((uint8_t*)dsdt_ptr,
                      (uint8_t*)dsdt_ptr,
                      sizeof(acpi_dsdt_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Verify checksum */
    sum = 0;

    kernel_serial_debug("DSDT Length \n");


    #ifdef DEBUG_ACPI
    kernel_serial_debug("DSDT Length %d\n", dsdt_ptr->header.length);
    #endif

    /* Mapping DSDT */
    err = kernel_mmap((uint8_t*)dsdt_ptr,
                      (uint8_t*)dsdt_ptr,
                      dsdt_ptr->header.length);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    for(i = 0; i < dsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)dsdt_ptr)[i];
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing END DSDT at 0x%08x\n", (uint32_t)dsdt_ptr);
    #endif

    if((sum & 0xFF) != 0)
    {
        kernel_error("DSDT Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    if(*((uint32_t*)dsdt_ptr->header.signature) != ACPI_DSDT_SIG)
    {
        kernel_error("DSDT Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing END DSDT at 0x%08x\n", (uint32_t)dsdt_ptr);
    #endif

    return OS_NO_ERR;
}

/* Parse the APIC FADT table.
 * The function will save the FADT table address in for further use. Then the
 * FACS and DSDT addresses are extracted and both tables are parsed.
 *
 * @param fadt_ptr The address of the FADT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_fadt(acpi_fadt_t* fadt_ptr)
{
    int32_t     sum;
    uint32_t    i;
    OS_RETURN_E err;

    if(fadt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing FADT at 0x%08x\n", (uint32_t)fadt_ptr);
    #endif

    /* Mapping FADT */
    err = kernel_mmap((uint8_t*)fadt_ptr,
                      (uint8_t*)fadt_ptr,
                      sizeof(acpi_fadt_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < fadt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)fadt_ptr)[i];
    }

    if((sum & 0xFF) != 0)
    {
        kernel_error("FADT Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    if(*((uint32_t*)fadt_ptr->header.signature) != ACPI_FACP_SIG)
    {
        kernel_error("FADT Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    /* Parse FACS */
    err = acpi_parse_facs((acpi_facs_t*)fadt_ptr->firmware_control);
    if(err == OS_NO_ERR)
    {
        facs_parse_success = 1;
        facs = (acpi_facs_t*)fadt_ptr->firmware_control;
    }
    else
    {
        kernel_error("Failed to parse FACS [%d]\n", err);
    }

    /* Parse DSDT */
    err =  acpi_parse_dsdt((acpi_dsdt_t*)fadt_ptr->dsdt);
    if(err == OS_NO_ERR)
    {
        dsdt_parse_success = 1;
        dsdt = (acpi_dsdt_t*)fadt_ptr->dsdt;
    }
    else
    {
        kernel_error("Failed to parse DSDT [%d]\n", err);
    }

    return OS_NO_ERR;
}

/* Parse the APIC SDT table.
 * The function will detect the SDT given as parameter thanks to the information
 * contained in the header. Then, if the entry is correctly detected and
 * supported, the parsing function corresponding will be called.
 *
 * @param header The address of the SDT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_dt(acpi_header_t* header)
{
    char sig_str[5];

    OS_RETURN_E err = OS_NO_ERR;

    if(header == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing SDT at 0x%08x\n", (uint32_t)header);
    #endif

    memcpy(sig_str, header->signature, 4);
    sig_str[4] = 0;

    if(*((uint32_t*)header->signature) == ACPI_FACP_SIG)
    {
        err = acpi_parse_fadt((acpi_fadt_t*)header);
        if(err == OS_NO_ERR)
        {
            fadt_parse_success = 1;
            fadt = (acpi_fadt_t *)header;
        }
        else
        {
            kernel_error("Failed to parse FADT [%d]\n", err);
        }
    }
    else if(*((uint32_t*)header->signature) == ACPI_APIC_SIG)
    {
        err = acpi_parse_apic((acpi_madt_t*)header);
        if(err == OS_NO_ERR)
        {
            madt_parse_success = 1;
            madt = (acpi_madt_t *)header;
        }
        else
        {
            kernel_error("Failed to parse MADT [%d]\n", err);
        }
    }

    return err;
}

/* Parse the APIC RSDT table.
 * The function will detect the read each entries of the RSDT and call the
 * corresponding functions to parse the entries correctly.
 *
 * @param rsdt_ptr The address of the RSDT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_rsdt(rsdt_descriptor_t* rsdt_ptr)
{
    uint32_t *range_begin;
    uint32_t *range_end;
    uint32_t address;
    int8_t   sum;
    uint8_t  i;

    OS_RETURN_E err = OS_NO_ERR;

    if(rsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing RSDT at 0x%08x\n", (uint32_t)rsdt_ptr);
    #endif

    /* Mapping RSDT */
    err = kernel_mmap((uint8_t*)rsdt_ptr,
                      (uint8_t*)rsdt_ptr,
                      sizeof(rsdt_descriptor_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < rsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)rsdt_ptr)[i];
    }

    if((sum & 0xFF) != 0)
    {
        kernel_printf("RSDT Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    if(*((uint32_t*)rsdt_ptr->header.signature) != ACPI_RSDT_SIG)
    {
        kernel_printf("RSDT Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    range_begin = (uint32_t*)(&rsdt_ptr->dt_pointers);
    range_end   = (uint32_t*)((uint8_t*)rsdt_ptr + rsdt_ptr->header.length);

    /* Parse each SDT of the RSDT */
    while(range_begin < range_end)
    {
        address = *range_begin;

        #ifdef DEBUG_ACPI
        kernel_serial_debug("Parsing SDT at 0x%08x\n", (uint32_t)address);
        #endif

        err = acpi_parse_dt((acpi_header_t*)address);

        if(err != OS_NO_ERR)
        {
            kernel_error("ACPI DT Parse error[%d]\n", err);
            break;
        }
        ++range_begin;
    }

    return err;
}

/* Parse the APIC XSDT table.
 * The function will detect the read each entries of the XSDT and call the
 * corresponding functions to parse the entries correctly.
 *
 * @param xsdt_ptr The address of the XSDT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_xsdt(xsdt_descriptor_t* xsdt_ptr)
{
    uint64_t *range_begin;
    uint64_t *range_end;
    uint32_t address;
    int8_t   sum;
    uint8_t  i;

    OS_RETURN_E err = OS_NO_ERR;

    if(xsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing XSDT at 0x%08x\n", (uint32_t)xsdt_ptr);
    #endif

    /* Mapping XSDT */
    err = kernel_mmap((uint8_t*)xsdt_ptr,
                      (uint8_t*)xsdt_ptr,
                      sizeof(xsdt_descriptor_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < xsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)xsdt_ptr)[i];
    }

    if((sum & 0xFF) != 0)
    {
        kernel_printf("RSDT Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    if(*((uint32_t*)xsdt_ptr->header.signature) != ACPI_XSDT_SIG)
    {
        kernel_printf("XSDT Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    range_begin = (uint64_t*)(&xsdt_ptr->dt_pointers);
    range_end   = (uint64_t*)((uint8_t*)xsdt_ptr + xsdt_ptr->header.length);

    /* Parse each SDT of the XSDT */
    while(range_begin < range_end)
    {
        address = (uint32_t)*range_begin;

        #ifdef DEBUG_ACPI
        kernel_serial_debug("Parsing SDT at 0x%08x\n", (uint32_t)address);
        #endif

        err = acpi_parse_dt((acpi_header_t*)address);

        if(err != OS_NO_ERR)
        {
            kernel_error("ACPI DT Parse error[%d]\n", err);
            break;
        }
        ++range_begin;
    }

    return err;
}

/* Use the APIC RSDP to parse the ACPI infomation. The function will detect the
 * RSDT or XSDT pointed and parse them.
 *
 * @param rsdp_desc The RSDP to walk.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_rsdp(rsdp_descriptor_t* rsdp_desc)
{
    uint8_t              sum;
    uint8_t              i;
    uint64_t             xsdt_addr;
    OS_RETURN_E          err;
    rsdp_descriptor_2_t* extended_rsdp;

    if(rsdp_desc == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("Parsing RSDP at 0x%08x\n", (uint32_t)rsdp_desc);
    #endif

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < sizeof(rsdp_descriptor_t); ++i)
    {
        sum += ((uint8_t*)rsdp_desc)[i];
    }

    if((sum & 0xFF) != 0)
    {
        kernel_printf("RSDP Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    #ifdef DEBUG_ACPI
    kernel_serial_debug("ACPI revision %d detected \n", rsdp_desc->revision);
    #endif

    /* ACPI version check */
    if(rsdp_desc->revision == 0)
    {

        err = acpi_parse_rsdt((rsdt_descriptor_t*)rsdp_desc->rsdt_address);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        rsdt_parse_success = 1;
    }
    else if(rsdp_desc->revision == 2)
    {
        extended_rsdp = (rsdp_descriptor_2_t*) rsdp_desc;
        sum = 0;

        for(i = 0; i < sizeof(rsdp_descriptor_2_t); ++i)
        {
            sum += ((uint8_t*)extended_rsdp)[i];
        }

        if(sum != 0)
        {
            kernel_printf("Extended RSDP Checksum failed\n");
            return OS_ERR_CHECKSUM_FAILED;
        }

        xsdt_addr = extended_rsdp->xsdt_address;

        if(xsdt_addr)
        {
            err = acpi_parse_xsdt((xsdt_descriptor_t*)(uint32_t)xsdt_addr);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            xsdt_parse_success = 1;
            has_xsdt = 1;
        }
        else
        {
            err = acpi_parse_rsdt((rsdt_descriptor_t*)rsdp_desc->rsdt_address);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            rsdt_parse_success = 1;
        }


    }
    else
    {
        kernel_printf("Unsupported ACPI version %d\n", rsdp_desc->revision);
        return OS_ERR_ACPI_UNSUPPORTED;
    }

    return OS_NO_ERR;
}

OS_RETURN_E init_acpi(void)
{
    uint8_t* range_begin;
    uint8_t* range_end;
    uint64_t signature;
    uint8_t  i;
    OS_RETURN_E err = OS_NO_ERR;

    /* Init pointers */
    has_xsdt = 0;
    rsdp = NULL;
    rsdt = NULL;
    xsdt = NULL;
    fadt = NULL;
    facs = NULL;
    dsdt = NULL;
    madt = NULL;

    /* Init data */
    rsdp_parse_success = 0;
    xsdt_parse_success = 0;
    rsdt_parse_success = 0;
    fadt_parse_success = 0;
    facs_parse_success = 0;
    dsdt_parse_success = 0;
    madt_parse_success = 0;

    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
        cpu_lapic[i] = NULL;
        cpu_ids[i] = 0;
    }

    for(i = 0; i < MAX_IO_APIC_COUNT; ++i)
    {
        io_apic_tables[i] = NULL;
    }

    cpu_count     = 0;
    io_apic_count = 0;

    /* Define ACPI table search address range */
    range_begin = (uint8_t*)0x000E0000;
    range_end   = (uint8_t*)0x000FFFFF;

    /* Search for ACPI table */
    while (range_begin < range_end)
    {
        signature = *(uint64_t*)range_begin;

        /* Checking the RSDP signature */
        if(signature == ACPI_RSDP_SIG)
        {
            #ifdef DEBUG_ACPI
            kernel_serial_debug("ACPI RSDP found at 0x%08x\n",
                                (uint32_t)range_begin);
            #endif

            /* Parse RSDP */
            err = acpi_parse_rsdp((rsdp_descriptor_t*)range_begin);
            if(err == OS_NO_ERR)
            {
                rsdp = (rsdp_descriptor_2_t*)range_begin;
                rsdp_parse_success = 1;
                break;
            }
        }

        range_begin += sizeof(uint64_t);
    }

    return err;
}

int8_t acpi_get_io_apic_available(void)
{
    if(acpi_init != 1)
    {
        return -1;
    }

    return ((acpi_get_lapic_available()) && io_apic_count > 0);
}

int8_t acpi_get_lapic_available(void)
{
    if(acpi_init != 1)
    {
        return -1;
    }

    return cpu_count > 0;
}

int32_t acpi_get_remmaped_irq(const uint8_t irq_number)
{
    uint8_t* base;
    uint8_t* limit;
    apic_interrupt_override_t* int_override;
    apic_header_t*             header;

    if(acpi_init != 1)
    {
        return -1;
    }

    if(madt_parse_success == 0)
    {
        return irq_number;
    }

    base  = (uint8_t*)(madt + 1);
    limit = ((uint8_t*) madt) + madt->header.length;

    /* Walk the table */
    while (base < limit)
    {
        header = (apic_header_t*)base;

        /* Check for type */
        if (header->type == APIC_TYPE_INTERRUPT_OVERRIDE)
        {
            int_override = (apic_interrupt_override_t*)base;

            /* Return remaped IRQ number */
            if (int_override->source == irq_number)
            {
                #ifdef DEBUG_ACPI
                kernel_serial_debug("ACPI Interrupt override found %d -> %d\n",
                                    int_override->source,
                                    int_override->interrupt);
                #endif

                return int_override->interrupt;
            }
        }

        base += header->length;
    }

    return irq_number;
}

uint8_t* acpi_get_io_apic_address(void)
{
    if(acpi_init != 1)
    {
        return NULL;
    }

    if(madt_parse_success == 0)
    {
        return NULL;
    }

    return (uint8_t*)io_apic_tables[0]->io_apic_addr;
}

uint8_t* get_lapic_addr(void)
{
    if(acpi_init != 1)
    {
        return NULL;
    }

    if(madt_parse_success == 0)
    {
        return NULL;
    }

    return (uint8_t*)madt->local_apic_addr;
}

OS_RETURN_E acpi_check_lapic_id(const uint32_t lapic_id)
{
    uint32_t i;

    if(acpi_init != 1)
    {
        return OS_ACPI_NOT_INITIALIZED;
    }

    for(i = 0; i < cpu_count; ++i)
    {
        if(cpu_lapic[i]->apic_id == lapic_id)
        {
            return OS_NO_ERR;
        }
    }

    return OS_ERR_NO_SUCH_LAPIC_ID;
}
