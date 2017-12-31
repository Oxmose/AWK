#include "../drivers/ata.h"
#include "../lib/string.h"
#include "../lib/stdio.h"

int test_ata(void)
{
    ata_device_t dev;
    dev.port = PRIMARY_PORT;
    dev.type = MASTER;

    char buffer[35] = "THIS IS A TEST FOR PATHOS TEST TEST";
    char recv[36] = {0};
    if(ata_write_sector(dev, 500, (uint8_t*)buffer, 35) != OS_NO_ERR)
    {
        printf("Failed to write\n");
        return 1;
    }
    if(ata_write_sector(dev, 600, (uint8_t*)buffer, 35) != OS_NO_ERR)
    {
        printf("Failed to write\n");
        return 1;
    }

    if(ata_read_sector(dev, 600, (uint8_t*)recv, 35) != OS_NO_ERR)
    {
        printf("Failed to read\n");
        return 1;
    }
    printf("READ: %s\n", recv);
    if(strncmp(recv, buffer, 22) != 0)
    {
        printf("Failed to compare\n");
        return 1;
    }


    memset(recv, 0, 36);
    if(ata_read_sector(dev, 500, (uint8_t*)recv, 35) != OS_NO_ERR)
    {
        printf("Failed to read\n");
        return 1;
    }
    printf("READ: %s\n", recv);
    if(strncmp(recv, buffer, 35) != 0)
    {
        printf("Failed to compare\n");
        return 1;
    }

    return 0;
}