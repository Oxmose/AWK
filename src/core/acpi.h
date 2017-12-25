/*******************************************************************************
 *
 * File: acpi.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 22/12/2017
 *
 * Version: 1.0
 *
 * Kernel ACPI management.
 ******************************************************************************/

#ifndef __ACPI_H_
#define __ACPI_H_

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */

/***********************************
 * CONSTANTS
 **********************************/
#define MAX_IO_APIC_COUNT            16

/* APIC structure types */
#define APIC_TYPE_LOCAL_APIC         0
#define APIC_TYPE_IO_APIC            1
#define APIC_TYPE_INTERRUPT_OVERRIDE 2
#define APIC_TYPE_NMI                4

/* ACPI SIGNATURE */
#define ACPI_RSDP_SIG 0x2052545020445352
#define ACPI_RSDT_SIG 0x54445352
#define ACPI_XSDT_SIG 0x54445358
#define ACPI_FACP_SIG 0x50434146
#define ACPI_FACS_SIG 0x53434146
#define ACPI_APIC_SIG 0x43495041
#define ACPI_DSDT_SIG 0x54445344

/***********************************
 * STRUCTURES
 **********************************/
typedef struct acpi_header
{
    char        signature[4];
    uint32_t    length;
    uint8_t     revision;
    uint8_t     checksum;

    char        oem[6];
    char        oem_table_id[8];
    uint32_t    oem_revision;

    uint32_t    creator_id;
    uint32_t    creator_revision;
} __attribute__((__packed__)) acpi_header_t;

typedef struct rsdp_descriptor 
{
    char            signature[8];
    uint8_t         checksum;
    char            oemid[6];
    uint8_t         revision;
    uint32_t        rsdt_address;
} __attribute__ ((packed)) rsdp_descriptor_t;

typedef struct rsdp_descriptor_2
{
    rsdp_descriptor_t first_part;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} __attribute__ ((packed)) rsdp_descriptor_2_t;

typedef struct rsdt_descriptor
{
    acpi_header_t header;
    uint32_t      *dt_pointers;
} __attribute__ ((packed)) rsdt_descriptor_t;

typedef struct xsdt_descriptor
{
    acpi_header_t header;
    uint64_t      *dt_pointers;
} __attribute__ ((packed)) xsdt_descriptor_t;

typedef struct generic_address
{
  uint8_t   address_space;

  uint8_t   bit_width;
  uint8_t   bit_offset;

  uint8_t   access_size;

  uint64_t  address;
} __attribute__((__packed__)) generic_address_t;

typedef struct acpi_fadt
{
    acpi_header_t      header;

    uint32_t            firmware_control;
    uint32_t            dsdt;

    uint8_t             reserved0;

    uint8_t             preferred_pm_profile;
    uint16_t            sci_interrupt;
    uint32_t            smi_command_port;

    uint8_t             acpi_enable;
    uint8_t             acpi_disable;

    uint8_t             S4BIOS_req;
    uint8_t             PSTATE_control;

    uint32_t            PM1_a_event_block;
    uint32_t            PM1_b_event_block;

    uint32_t            PM1_a_control_block;
    uint32_t            PM1_b_control_block;

    uint32_t            PM2_control_block;

    uint32_t            PM_timer_block;

    uint32_t            GPE0_block;
    uint32_t            GPE1_block;

    uint8_t             PM1_event_length;

    uint8_t             PM1_control_length;
    uint8_t             PM2_control_length;

    uint8_t             PM_timer_length;

    uint8_t             GPE0_length;
    uint8_t             GPE1_length;

    uint8_t             GPE1_base;

    uint8_t             C_state_control;

    uint16_t            worst_C2_latency;
    uint16_t            worst_C3_latency;

    uint16_t            flush_size;
    uint16_t            flush_stride;

    uint8_t             duty_offset;
    uint8_t             duty_width;

    uint8_t             day_alarm;
    uint8_t             month_alarm;

    uint8_t             century;
 
    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t            boot_architecture_flags;
 
    uint8_t             reserved1;

    uint32_t            flags;
 
    // 12 byte structure; see below for details
    generic_address_t   reset_reg;
 
    uint8_t             reset_value;

    uint8_t             reserved2[3];
 
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t            X_firmware_control;
    uint64_t            X_dsdt;
 
    generic_address_t   X_PM1_a_event_block;
    generic_address_t   X_PM1_b_event_block;

    generic_address_t   X_PM1_a_control_block;
    generic_address_t   X_PM1_b_control_block;

    generic_address_t   X_PM2_control_block;

    generic_address_t   X_PM_timer_block;

    generic_address_t   X_GPE0_block;
    generic_address_t   X_GPE1_block;
    
} __attribute__((__packed__)) acpi_fadt_t;

typedef struct acpi_facs
{
    acpi_header_t      header;

    /* TODO */
}  __attribute__((__packed__)) acpi_facs_t;

typedef struct acpi_dsdt
{
    acpi_header_t      header;

    /* TODO */
}  __attribute__((__packed__)) acpi_dsdt_t;

typedef struct acpi_madt
{
    acpi_header_t   header;

    uint32_t            local_apic_addr;
    uint32_t            flags;
} __attribute__((__packed__)) acpi_madt_t;

typedef struct apic_header
{
    uint8_t type;
    uint8_t length;
} __attribute__((__packed__)) apic_header_t;

typedef struct io_apic
{
    apic_header_t   header;

    uint8_t         apic_id;

    uint8_t         reserved;

    uint32_t        io_apic_addr;
    uint32_t        global_system_interrupt_base;
} __attribute__((__packed__)) io_apic_t;

typedef struct local_apic
{
    apic_header_t   header;

    uint8_t         acpi_cpu_id;
    uint8_t         apic_id;
    uint32_t        flags;
} __attribute__((__packed__)) local_apic_t;

typedef struct apic_interrupt_override
{
    apic_header_t   header;

    uint8_t         bus;
    uint8_t         source;
    uint32_t        interrupt;
    uint16_t        flags;
} __attribute__((__packed__)) apic_interrupt_override_t;

typedef struct local_apic_nmi
{
    uint8_t         processors;
    uint16_t        flags;
    uint8_t         lint_id;
} __attribute__((__packed__)) local_apic_nmi_t;

/***********************************
 * FUNCTIONS
 **********************************/

OS_RETURN_E init_acpi(void);

#endif /* __ACPI_H_ */