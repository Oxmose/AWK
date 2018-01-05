#include "../lib/stdint.h" /* Generic int types */
#include "../cpu/bios_call.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../drivers/vesa.h"
#include "../drivers/ata.h"
#include "../lib/malloc.h"
#include "../core/scheduler.h"

int test_vesa(void)
{

	ata_device_t dev;
	dev.port = PRIMARY_PORT;
	dev.type = MASTER;

 	uint32_t sector = 0;
	uint8_t buffer[512];
 	uint32_t index = 512;
 	uint8_t  pixel[4] = {0};

 	for(int m = 0; m < 960; ++m)
 	{
 		for(int n = 0; n < 1280; ++n)
 		{
 			if(index + 2 >= 512)
 			{
 				/* Consume the rest */
 				int tmp = 2;
 				while(index < 512)
 				{
 					pixel[tmp] = buffer[index];
 					++index;
 					--tmp;
 				}
 				if(ata_read_sector(dev, sector, (uint8_t*)buffer, 512) != OS_NO_ERR)
			    {
			        printf("Failed to read\n");
			        return -1;
			    }
			    ++sector;
			    index = 0;

			    /* Get the rest of the pixel */
			    while(tmp >= 0)
			    {
			    	pixel[tmp] = buffer[index];
 					++index;
 					--tmp;
			    }
 			}
 			else
 			{
	 			pixel[0] = buffer[index+2];  // B
	 			pixel[1] = buffer[index+1];  // G
	 			pixel[2] = buffer[index];    // R
	 			pixel[3] = 0;
	 			index += 3;
	 		}

 			vesa_draw_pixel(n, m, pixel[2], pixel[1], pixel[0]);
 		}
 	}
	return 0;
}