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

#include "../debug.h"      /* kernel_serial_debug */

/* Header file */
#include "ata.h"

/********************************
 * FUNCTIONS
 *******************************/
OS_RETURN_E init_ata(void)
{
	ata_device_t device;

	device.port = PRIMARY_PORT;
	device.type = MASTER;
	ata_identify_device(device);
	device.type = SLAVE;
	ata_identify_device(device);


	device.port = SECONDARY_PORT;
	device.type = MASTER;
	ata_identify_device(device);
	device.type = SLAVE;
	ata_identify_device(device);

	device.port = THIRD_PORT;
	device.type = MASTER;
	ata_identify_device(device);
	device.type = SLAVE;
	ata_identify_device(device);

	device.port = FOURTH_PORT;
	device.type = MASTER;
	ata_identify_device(device);
	device.type = SLAVE;
	ata_identify_device(device);

	return OS_NO_ERR;
}

OS_RETURN_E ata_identify_device(ata_device_t device)
{
	uint8_t status;
	uint16_t i;
	uint16_t data;
	char ata_str[513] = {0};
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
    while(((status & ATA_FLAG_BUSY) == ATA_FLAG_BUSY)
       && ((status & ATA_FLAG_ERR) != ATA_FLAG_ERR))
    {
    	status = inb(device.port + ATA_COMMAND_PORT_OFFSET);
    }
        
    if((status & ATA_FLAG_ERR) == 1)
    {
    	#ifdef DEBUG_ATA
	    kernel_serial_debug("ATA device error\n");
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
    #ifdef DEBUG_ATA
    kernel_serial_debug("ATA STR: %s\n", ata_str);
    #endif
    
    return OS_NO_ERR;
}

OS_RETURN_E ata_read(ata_device_t device, uint32_t sector, uint8_t* buffer, uint32_t size)
{
	(void)device;
	(void)sector;
	(void)buffer;
	(void)size;
	return OS_NO_ERR;
}

OS_RETURN_E ata_write(ata_device_t device, uint32_t sector, const uint8_t* buffer, uint32_t size)
{
	(void)device;
	(void)sector;
	(void)buffer;
	(void)size;
	return OS_NO_ERR;
}

OS_RETURN_E ata_flush(ata_device_t device)
{
	(void)device;
	return OS_NO_ERR;
}