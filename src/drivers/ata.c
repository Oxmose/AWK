/*******************************************************************************
 *
 * File: ata.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.5
 *
 * Advanced Technology Attachment driver for the kernel.
 ******************************************************************************/

#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../cpu/cpu.h"            /* oub inb */
#include "../core/kernel_output.h" /* kernel_info kernel_error */

#include "../debug.h"      /* kernel_serial_debug */

/* Header file */
#include "ata.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E init_ata(void)
{
    ata_device_t device;
    OS_RETURN_E  err;

    /* Check for all devices */
    device.port = PRIMARY_PORT;
    device.type = MASTER;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        kernel_error("ATA device %s at 0x%x error [%d]\n",
                                                     ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port,
                                                     err);
    }
    device.type = SLAVE;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        kernel_error("ATA device %s at 0x%x error [%d]\n",
                                                     ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port,
                                                     err);
    }

    device.port = SECONDARY_PORT;
    device.type = MASTER;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        kernel_error("ATA device %s at 0x%x error [%d]\n",
                                                     ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port,
                                                     err);
    }
    device.type = SLAVE;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        kernel_error("ATA device %s at 0x%x error [%d]\n",
                                                     ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port,
                                                     err);
    }

    device.port = THIRD_PORT;
    device.type = MASTER;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        kernel_error("ATA device %s at 0x%x error [%d]\n",
                                                     ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port,
                                                     err);
    }
    device.type = SLAVE;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        kernel_error("ATA device %s at 0x%x error [%d]\n",
                                                     ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port,
                                                     err);
    }

    device.port = FOURTH_PORT;
    device.type = MASTER;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        kernel_error("ATA device %s at 0x%x error [%d]\n",
                                                     ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port,
                                                     err);
    }
    device.type = SLAVE;
    err = ata_identify_device(device);
    if(err == OS_NO_ERR)
    {
        kernel_info("Found ATA device %s at 0x%x\n", ((device.type == MASTER) ?
                                                     "MASTER" : "SLAVE"),
                                                     device.port);
    }
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        return err;
    }

    return OS_NO_ERR;
}

OS_RETURN_E ata_identify_device(ata_device_t device)
{
    uint8_t  status;
    uint16_t i;
    uint16_t data;
    char     ata_str[513] = {0};
    uint16_t ata_index;

    #ifdef DEBUG_ATA
    kernel_serial_debug("IDENTIFY ATA 0x%08x %s\n",
                        device.port,
                        ((device.type == MASTER) ? "MASTER" : "SLAVE"));
    #endif

    /* Select slave or master */
    outb(device.type == MASTER ? 0xA0 : 0xB0,
         device.port + ATA_DEVICE_PORT_OFFSET);

    /* Check is the device is connected */
    outb(0x00, device.port + ATA_CONTROL_PORT_OFFSET);

    status = inb(device.port + ATA_COMMAND_PORT_OFFSET);
    if(status == 0xFF)
    {
        #ifdef DEBUG_ATA
        kernel_serial_debug("ATA device not present\n");
        #endif
        return OS_ERR_ATA_DEVICE_NOT_PRESENT;
    }

    /* Select slave or master */
    outb(device.type == MASTER ? 0xA0 : 0xB0,
         device.port + ATA_DEVICE_PORT_OFFSET);

    /* Write 0 to registers */
    outb(0x00, device.port + ATA_SC_PORT_OFFSET);
    outb(0x00, device.port + ATA_LBALOW_PORT_OFFSET);
    outb(0x00, device.port + ATA_LBAMID_PORT_OFFSET);
    outb(0x00, device.port + ATA_LBAHIG_PORT_OFFSET);

    /* Send the identify command */
    outb(ATA_IDENTIFY_COMMAND, device.port + ATA_COMMAND_PORT_OFFSET);

    /* Get the IDENTIFY status */
    status = inb(device.port + ATA_COMMAND_PORT_OFFSET);
    if(status == 0x00)
    {
        #ifdef DEBUG_ATA
        kernel_serial_debug("ATA device not present\n");
        #endif
          return OS_ERR_ATA_DEVICE_NOT_PRESENT;
    }

    /* Wait until device is ready */
    while(((status & ATA_FLAG_BUSY) == ATA_FLAG_BUSY) &&
          ((status & ATA_FLAG_ERR) != ATA_FLAG_ERR))
    {
        status = inb(device.port + ATA_COMMAND_PORT_OFFSET);
    }

    if((status & ATA_FLAG_ERR) == 1)
    {
        #ifdef DEBUG_ATA
        kernel_serial_debug("ATA device error 0x%08x (%s)\n", device.port,
                                                  ((device.type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
        #endif
        return OS_ERR_ATA_DEVICE_ERROR;
    }

    /* The device data information is now ready to be read */
    ata_index = 0;
    for(i = 0; i < 256; ++i)
    {
        data = inw(device.port + ATA_DATA_PORT_OFFSET);
        ata_str[ata_index++] = (data >> 8) & 0xFF;
        ata_str[ata_index++] = data & 0xFF;
    }
    (void)ata_str;

    #ifdef DEBUG_ATA
    kernel_serial_debug("ATA STR: %s\n", ata_str);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E ata_read_sector(ata_device_t device, const uint32_t sector,
                            uint8_t* buffer, const uint32_t size)
{
    uint32_t i;
    uint16_t data;
    uint8_t  status;

    #ifdef DEBUG_ATA
    kernel_serial_debug("ATA read request device 0x%08x %s, sector 0x%08x,\
size %d\n", device.port, ((device.type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
    #endif

    /* Check sector */
    if(sector > 0x0FFFFFFF)
    {
        return OS_ERR_ATA_BAD_SECTOR_NUMBER;
    }

    /* Check read size */
    if(size > ATA_SECTOR_SIZE)
    {
        return OS_ERR_ATA_SIZE_TO_HUGE;
    }

    /* Set sector to read */
    outb((device.type == MASTER ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24) ,
         device.port + ATA_DEVICE_PORT_OFFSET);

    /* Clear error */
    outb(0, device.port + ATA_ERROR_PORT_OFFSET);

    /* Set number of sector to read */
    outb(1, device.port + ATA_SC_PORT_OFFSET);

    /* Set LBA values */
    outb(sector & 0x000000FF, device.port + ATA_LBALOW_PORT_OFFSET);
    outb((sector & 0x0000FF00) >> 8, device.port + ATA_LBAMID_PORT_OFFSET);
    outb((sector & 0x00FF0000) >> 16, device.port + ATA_LBAHIG_PORT_OFFSET);

    /* Send Read sector command */
    outb(ATA_READ_SECTOR_COMMAND, device.port + ATA_COMMAND_PORT_OFFSET);

    /* Wait until device is ready */
    status = inb(device.port + ATA_COMMAND_PORT_OFFSET);
    while(((status & ATA_FLAG_BUSY) == ATA_FLAG_BUSY)
       && ((status & ATA_FLAG_ERR) != ATA_FLAG_ERR))
    {
        status = inb(device.port + ATA_COMMAND_PORT_OFFSET);
    }

    /* Check error status */
    if((status & ATA_FLAG_ERR) == ATA_FLAG_ERR)
    {
        #ifdef DEBUG_ATA
        kernel_serial_debug("ATA device read error 0x%08x (%s)\n", device.port,
                                                  ((device.type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
        #endif
        return OS_ERR_ATA_DEVICE_ERROR;
    }

       #ifdef DEBUG_ATA
    kernel_serial_debug("ATA read device 0x%08x %s, sector 0x%08x, size %d\n",
                        device.port,
                        ((device.type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
    #endif

    /* Read data and copy to buffer */
    for(i = 0; i < size; i += 2)
    {
        data = inw(device.port + ATA_DATA_PORT_OFFSET);

        buffer[i] = data & 0xFF;

        if(i + 1 < size)
        {
            buffer[i + 1] = (data >> 8) & 0xFF;
        }
    }

    /* Read the rest of the sector to release the memoey for next command */
    for(i = size + (size % 2); i < ATA_SECTOR_SIZE; i += 2)
    {
        inw(device.port + ATA_DATA_PORT_OFFSET);
    }

    return OS_NO_ERR;
}

OS_RETURN_E ata_write_sector(ata_device_t device, const uint32_t sector,
                             const uint8_t* buffer, const uint32_t size)
{
    uint32_t i;
    uint16_t data;

    #ifdef DEBUG_ATA
    kernel_serial_debug("ATA write request device 0x%08x %s, sector 0x%08x,\
size %d\n", device.port, ((device.type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
    #endif

    /* Check sector */
    if(sector > 0x0FFFFFFF)
    {
        return OS_ERR_ATA_BAD_SECTOR_NUMBER;
    }

    /* Check write size */
    if(size > ATA_SECTOR_SIZE)
    {
        return OS_ERR_ATA_SIZE_TO_HUGE;
    }

    /* Set sector to write */
    outb((device.type == MASTER ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24) ,
         device.port + ATA_DEVICE_PORT_OFFSET);

    /* Clear error */
    outb(0, device.port + ATA_ERROR_PORT_OFFSET);

    /* Set number of sector to write */
    outb(1, device.port + ATA_SC_PORT_OFFSET);

    /* Set LBA values */
    outb(sector & 0x000000FF, device.port + ATA_LBALOW_PORT_OFFSET);
    outb((sector & 0x0000FF00) >> 8, device.port + ATA_LBAMID_PORT_OFFSET);
    outb((sector & 0x00FF0000) >> 16, device.port + ATA_LBAHIG_PORT_OFFSET);

    /* Send write sector command */
    outb(ATA_WRITE_SECTOR_COMMAND, device.port + ATA_COMMAND_PORT_OFFSET);

    #ifdef DEBUG_ATA
    kernel_serial_debug("ATA write device 0x%08x %s, sector 0x%08x, size %d\n",
                        device.port,
                        ((device.type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
    #endif

    /* Write to disk */
    for(i = 0; i < size; i += 2)
    {
        data = buffer[i] & 0x00FF;
        if(i + 1 < size)
        {
            data |= ((uint16_t)buffer[i + 1]) << 8;
        }
        outw(data, device.port + ATA_DATA_PORT_OFFSET);
    }

    /* Add padding to the sector, maybe we should avoid this and rewrite
     * old data instead, TODO */
    for(i = size + (size % 2); i < ATA_SECTOR_SIZE; i += 2)
    {
        outw(0x0000, device.port + ATA_DATA_PORT_OFFSET);
    }

    /* Flush write */
    return ata_flush(device);
}

OS_RETURN_E ata_flush(ata_device_t device)
{
    uint8_t status;

    #ifdef DEBUG_ATA
    kernel_serial_debug("ATA flush request device 0x%08x %s\n",
                        device.port,
                        ((device.type == MASTER) ? "MASTER" : "SLAVE"));
    #endif

    /* Set device */
    outb((device.type == MASTER ? 0xE0 : 0xF0),
         device.port + ATA_DEVICE_PORT_OFFSET);

    /* Send write sector command */
    outb(ATA_FLUSH_SECTOR_COMMAND, device.port + ATA_COMMAND_PORT_OFFSET);

    /* Wait until device is ready */
    status = inb(device.port + ATA_COMMAND_PORT_OFFSET);

    /* Check error status */
    if(status == 0)
    {
        #ifdef DEBUG_ATA
        kernel_serial_debug("ATA flush write error 0x%08x (%s)\n", device.port,
                                                  ((device.type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
        #endif

        return OS_ERR_ATA_DEVICE_ERROR;
    }

    while(((status & ATA_FLAG_BUSY) == ATA_FLAG_BUSY)
       && ((status & ATA_FLAG_ERR) != ATA_FLAG_ERR))
    {
        status = inb(device.port + ATA_COMMAND_PORT_OFFSET);
    }

    /* Check error status */
    if((status & ATA_FLAG_ERR) == 1)
    {
        #ifdef DEBUG_ATA
        kernel_serial_debug("ATA flush write error 0x%08x (%s)\n", device.port,
                                                  ((device.type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
        #endif

        return OS_ERR_ATA_DEVICE_ERROR;
    }

    return OS_NO_ERR;
}