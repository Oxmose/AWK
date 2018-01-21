/*******************************************************************************
 *
 * File: test_ata.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: ATA PIO drivers tests
 ******************************************************************************/

#include "../../drivers/ata.h"
#include "../../core/kernel_output.h"
#include "../../core/panic.h"
#include "../../lib/string.h"

void test_ata(void)
{
    ata_device_t dev;
    dev.port = PRIMARY_PORT;
    dev.type = MASTER;

    char buffer[36] = "THIS IS A TEST FOR WAK  TEST  TEST \0";
    char recv[36] = {0};
    if(ata_write_sector(dev, 500, (uint8_t*)buffer, 35) != OS_NO_ERR)
    {
        kernel_error("Failed to write\n");
        kernel_panic();
    }
    if(ata_write_sector(dev, 600, (uint8_t*)buffer, 35) != OS_NO_ERR)
    {
        kernel_error("Failed to write\n");
        kernel_panic();
    }

    if(ata_read_sector(dev, 600, (uint8_t*)recv, 35) != OS_NO_ERR)
    {
        kernel_error("Failed to read\n");
        kernel_panic();
    }
    if(strncmp(recv, buffer, 35) != 0)
    {
        recv[35] = 0;
        buffer[35] = 0;
        kernel_error("Failed to compare\n --> %s \n --> %s\n", buffer, buffer);
        kernel_panic();
    }

    memset(recv, 0, 36);
    if(ata_read_sector(dev, 500, (uint8_t*)recv, 35) != OS_NO_ERR)
    {
        kernel_error("Failed to read\n");
        kernel_panic();
    }
    if(strncmp(recv, buffer, 35) != 0)
    {
        recv[35] = 0;
        buffer[35] = 0;
        kernel_error("Failed to compare\n --> %s \n --> %s\n", buffer, buffer);
        kernel_panic();
    }

    kernel_debug("ATA tests passed\n");
}
