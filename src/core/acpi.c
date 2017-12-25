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

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/string.h" /* memcpy */
#include "../cpu/cpu.h"    /* MAX_CPU_COUNT */
#include "kernel_output.h"

/* Header file */
#include "acpi.h"

/* CPU informations */
static uint32_t cpu_count;
static uint32_t cpu_ids[MAX_CPU_COUNT];

/* CPU LAPIC */
static uint8_t cpu_lapic_parse_success[MAX_CPU_COUNT];
static local_apic_t *cpu_lapic[MAX_CPU_COUNT];

/* IO APIC */
static uint32_t io_apic_count;
static io_apic_t *io_apic_tables[MAX_IO_APIC_COUNT];

/* ACPI Tables pointers */
static uint8_t rsdp_parse_success;
static rsdp_descriptor_2_t *rsdp;

static uint8_t has_xsdt;
static uint8_t rsdt_parse_success;
static rsdt_descriptor_t *rsdt;
static uint8_t xsdt_parse_success;
static xsdt_descriptor_t *xsdt;

static uint8_t fadt_parse_success;
static acpi_fadt_t     *fadt;

static uint8_t facs_parse_success;
static acpi_facs_t     *facs;

static uint8_t dsdt_parse_success;
static acpi_dsdt_t     *dsdt;

static uint8_t madt_parse_success;
static acpi_madt_t *madt;

static OS_RETURN_E acpi_parse_apic(acpi_madt_t *madt_ptr)
{
    int32_t sum;
    uint32_t i;
    uint8_t *madt_entry;
    uint8_t *madt_limit;
    uint8_t type;

    if(madt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
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

    madt_entry = (uint8_t *)(madt_ptr + 1);
    madt_limit = ((uint8_t *)madt_ptr) + madt_ptr->header.length;

    cpu_count = 0;
    io_apic_count = 0;
    while (madt_entry < madt_limit)
    {
        /* Get entry header */
        apic_header_t *header = (apic_header_t *)madt_entry;
        type = header->type;

        /* Check entry type */
        if(type == APIC_TYPE_LOCAL_APIC)
        {
            if(cpu_count < MAX_CPU_COUNT)
            {
                /* Add CPU info to the lapic table */
                cpu_lapic[cpu_count] = (local_apic_t *)madt_entry;
                cpu_ids[cpu_count] = cpu_lapic[cpu_count]->acpi_cpu_id;
                ++cpu_count;
            }
            else
            {
                kernel_info("Exceeded CPU max count, ignoring.\n");
                /* TODO IDLE CPU */
            }
        }
        else if(type == APIC_TYPE_IO_APIC)
        {
            if(io_apic_count < MAX_IO_APIC_COUNT)
            {
                /* Add IO APIC info to the table */
                io_apic_tables[io_apic_count] = (io_apic_t *)madt_entry;
                ++io_apic_count;
            }
            else
            {
                kernel_info("Exceeded IO APIC max count, ignoring.\n");
            }
            
        }
        else if(type == APIC_TYPE_INTERRUPT_OVERRIDE)
        {
            //apic_interrupt_override_t *s = (apic_interrupt_override_t *)madt_entry;

            //kernel_printf("Found Interrupt Override: %d %d %d 0x%04x\n", s->bus, s->source, s->interrupt, s->flags);

            /* TODO */
        }
        else if(type == APIC_TYPE_NMI)
        {
            //local_apic_nmi_t *s = (local_apic_nmi_t *)madt_entry;

            //kernel_printf("Found NMI: 0x%2d 0x%4x %d\n", s->processors, s->flags, s->lint_id);
             /* TODO */
        }
        else
        {
            kernel_printf("Unknown APIC structure %d\n", type);
        }

        madt_entry += header->length;
    }

    return OS_NO_ERR;
}

static OS_RETURN_E acpi_parse_facs(acpi_facs_t *facs_ptr)
{
    int32_t sum;
    uint32_t i;

    if(facs_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < facs_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)facs_ptr)[i];
    }

    if((sum & 0xFF) != 0)
    {
        kernel_error("FACS Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    if(*((uint32_t*)facs_ptr->header.signature) != ACPI_FACS_SIG)
    {
        kernel_error("FACS Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    return OS_NO_ERR;
}

static OS_RETURN_E acpi_parse_dsdt(acpi_dsdt_t *dsdt_ptr)
{
    int32_t sum;
    uint32_t i;

    if(dsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < dsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)dsdt_ptr)[i];
    }

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

    return OS_NO_ERR;
}

static OS_RETURN_E acpi_parse_fadt(acpi_fadt_t *fadt_ptr)
{
    int32_t sum;
    uint32_t i;
    OS_RETURN_E err;

    if(fadt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
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
    err = acpi_parse_facs((acpi_facs_t *)fadt_ptr->firmware_control);
    if(err == OS_NO_ERR)
    {
        facs_parse_success = 1;
        facs = (acpi_facs_t *)fadt_ptr->firmware_control;
    }
    else
    {
        kernel_error("Failed to parse FACS [%d]\n", err);
    }

    /* Parse DSDT */    
    err =  acpi_parse_dsdt((acpi_dsdt_t *)fadt_ptr->dsdt);
    if(err == OS_NO_ERR)
    {
        dsdt_parse_success = 1;
        dsdt = (acpi_dsdt_t *)fadt_ptr->dsdt;
    }
    else
    {
        kernel_error("Failed to parse DSDT [%d]\n", err);
    }

    return OS_NO_ERR;
}

static OS_RETURN_E acpi_parse_dt(acpi_header_t *header)
{
    OS_RETURN_E err = OS_NO_ERR;
    char sig_str[5];

    if(header == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    memcpy(sig_str, header->signature, 4);
    sig_str[4] = 0;

    if(*((uint32_t*)header->signature) == ACPI_FACP_SIG)
    {
        err = acpi_parse_fadt((acpi_fadt_t *)header);
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
        err = acpi_parse_apic((acpi_madt_t *)header);
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
    else
    {
        //kernel_printf("RSDT Signature: %s 0x%x\n", sig_str, *((uint32_t*)header->signature));
    }
    return err;
}

static OS_RETURN_E acpi_parse_rsdt(rsdt_descriptor_t *rsdt_ptr)
{
    uint32_t *range_begin;
    uint32_t *range_end;
    uint32_t address;

    int8_t sum;
    uint8_t i;

    OS_RETURN_E err = OS_NO_ERR;

    if(rsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
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

    range_begin = (uint32_t *)(&rsdt_ptr->dt_pointers);
    range_end   = (uint32_t *)((uint8_t*)rsdt_ptr + rsdt_ptr->header.length);

    /* Parse each SDT of the RSDT */
    while(range_begin < range_end)
    {
        address = *range_begin;
        err = acpi_parse_dt((acpi_header_t *)address);
        
        if(err != OS_NO_ERR)
        {
            kernel_error("ACPI DT Parse error[%d]\n", err);
            break;
        }
        ++range_begin;
    }

    return err;
}

static OS_RETURN_E acpi_parse_xsdt(xsdt_descriptor_t *xsdt_ptr)
{
    uint64_t *range_begin;
    uint64_t *range_end;
    uint32_t address;

    int8_t sum;
    uint8_t i;

    OS_RETURN_E err = OS_NO_ERR;

    if(xsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
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

    range_begin = (uint64_t *)(&xsdt_ptr->dt_pointers);
    range_end   = (uint64_t *)((uint8_t*)xsdt_ptr + xsdt_ptr->header.length);

    /* Parse each SDT of the XSDT */
    while(range_begin < range_end)
    {
        address = (uint32_t)*range_begin;
        err = acpi_parse_dt((acpi_header_t *)address);
        
        if(err != OS_NO_ERR)
        {
            kernel_error("ACPI DT Parse error[%d]\n", err);
            break;
        }
        ++range_begin;
    }

    return err;
}

static OS_RETURN_E acpi_parse_rsdp(rsdp_descriptor_t *rsdp_desc)
{
    uint8_t sum;
    uint8_t i;
    char oem[7];
    OS_RETURN_E err;
    uint64_t xsdt_addr;

    rsdp_descriptor_2_t *extended_rsdp;

    if(rsdp_desc == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

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

    /* Print OEM */
    memcpy(oem, rsdp_desc->oemid, 6);
    oem[6] = '\0';
    kernel_info("ACPI OEMID: %s\n", oem);

    /* ACPI version check */
    if(rsdp_desc->revision == 0)
    {
        err = acpi_parse_rsdt((rsdt_descriptor_t *)rsdp_desc->rsdt_address);
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
            err = acpi_parse_xsdt((xsdt_descriptor_t *)(uint32_t)xsdt_addr);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            xsdt_parse_success = 1;
            has_xsdt = 1;
        }
        else
        {
            err = acpi_parse_rsdt((rsdt_descriptor_t *)rsdp_desc->rsdt_address);
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
    OS_RETURN_E err;
    uint8_t *range_begin;
    uint8_t *range_end;
    uint64_t signature;
    uint8_t i;

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
        cpu_lapic_parse_success[i] = 0;
        cpu_lapic[i] = NULL;
        cpu_ids[i] = 0;
    }

    for(i = 0; i < MAX_IO_APIC_COUNT; ++i)
    {
        io_apic_tables[i] = NULL;
    }

    cpu_count = 0;
    io_apic_count = 0;

    /* Define ACPI table search address range */
    range_begin = (uint8_t *)0x000e0000;
    range_end   = (uint8_t *)0x000fffff;   

    /* Search for ACPI table */
    while (range_begin < range_end)
    {
        signature = *(uint64_t*)range_begin;

        /* Checking the RSDP signature */
        if(signature == ACPI_RSDP_SIG)
        {
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