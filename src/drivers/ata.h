/*******************************************************************************
 *
 * File: ata.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.5
 *
 * Advanced Technology Attachment driver for the kernel.
 ******************************************************************************/

#ifndef __ATA_H_
#define __ATA_H_

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define ATA_PRIMARY_PORT_ADDRESS   0x1F0
#define ATA_SECONDARY_PORT_ADDRESS 0x170
#define ATA_THIRD_PORT_ADDRESS     0x1E8
#define ATA_FOURTH_PORT_ADDRESS    0x168

#define ATA_DATA_PORT_OFFSET    0x000
#define ATA_ERROR_PORT_OFFSET   0x001
#define ATA_SC_PORT_OFFSET      0x002
#define ATA_LBALOW_PORT_OFFSET  0x003
#define ATA_LBAMID_PORT_OFFSET  0x004
#define ATA_LBAHIG_PORT_OFFSET  0x005
#define ATA_DEVICE_PORT_OFFSET  0x006
#define ATA_COMMAND_PORT_OFFSET 0x007
#define ATA_CONTROL_PORT_OFFSET 0x206

#define ATA_IDENTIFY_COMMAND     0xEC
#define ATA_READ_SECTOR_COMMAND  0x20
#define ATA_WRITE_SECTOR_COMMAND 0x30
#define ATA_FLUSH_SECTOR_COMMAND 0xE7

#define ATA_FLAG_BUSY 0x80
#define ATA_FLAG_ERR  0x01

#define ATA_SECTOR_SIZE 512

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef enum ATA_TYPE
{
    MASTER = 0,
    SLAVE  = 1
} ATA_TYPE_E;

typedef enum ATA_PORT
{
    PRIMARY_PORT   = ATA_PRIMARY_PORT_ADDRESS,
    SECONDARY_PORT = ATA_SECONDARY_PORT_ADDRESS,
    THIRD_PORT     = ATA_THIRD_PORT_ADDRESS,
    FOURTH_PORT    = ATA_FOURTH_PORT_ADDRESS
} ATA_PORT_E;

typedef struct ata_device
{
    ATA_PORT_E port;
    ATA_TYPE_E type;
} ata_device_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init ATA settings
 *
 * @return The error or success state.
 */
OS_RETURN_E init_ata(void);

/* Identify the ATA device given as parameter. The function will check the
 * presence of a device conected to the port pointed by the device argument.
 *
 * @param device The device to identify.
 * @returns OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E ata_identify_device(ata_device_t device);

/* Read size bytes on the device pointer by the device given as parameter.
 * The number of bytes to read must be less of equal to the size of a sector.
 *
 * @param device The device to read the data from.
 * @param sector The sector number where the data are located.
 * @param buffer The buffer that is used to store the read data.
 * @param size The number of bytes to read from the device.
 * @returns OS_NO_ERR if no error were detected. An error is returned otherwise.
 */
OS_RETURN_E ata_read_sector(ata_device_t device, const uint32_t sector,
	                        uint8_t* buffer, const uint32_t size);

/* Write size bytes on the device pointer by the device given as parameter.
 * The number of bytes to written must be less of equal to the size of a sector.
 *
 * !!! The entire sector will be rewritten and padded with NULL values !!!
 *
 * @param device The device to write the data to.
 * @param sector The sector number where the data are to be written.
 * @param buffer The buffer containing the data to write.
 * @param size The number of bytes to read from the device.
 * @returns OS_NO_ERR if no error were detected. An error is returned otherwise.
 */
OS_RETURN_E ata_write_sector(ata_device_t device, const uint32_t sector,
	                         const uint8_t* buffer, const uint32_t size);

/* Ask the ATA device to flush the data cache. This is used to ensure correct
 * writing to the device.
 *
 * @param device The device to be written.
 * @returns OS_NO_ERR if no error were detected. An error is returned otherwise.
 */
OS_RETURN_E ata_flush(ata_device_t device);

#endif /* __ATA_H_ */