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
	OS_RETURN_E err;

	uint16_t vesa_mode_count = get_vesa_mode_count();
	vesa_mode_info_t* modes = malloc(sizeof(vesa_mode_info_t) * vesa_mode_count);


	err = get_vesa_modes(modes, vesa_mode_count);
	if(err != OS_NO_ERR)
	{
		printf("ERROR getting vesa mode %d\n", err);
		return -1;
	}

	for(uint32_t i = 0; i < vesa_mode_count; ++i)
	{
		printf("MODE %3d: %dx%d %dBits\n", modes[i].mode_id, modes[i].width, modes[i].height, modes[i].bpp);
	}

	for(uint32_t i = 0; i < vesa_mode_count; ++i)
	{
		if(modes[i].width == 1280 &&
	      	 modes[i].height == 800 &&
	      	 modes[i].bpp == 32)
	    {
	    	ata_device_t dev;
			dev.port = PRIMARY_PORT;
			dev.type = MASTER;


	      	err = set_vesa_mode(modes[i]);
	      	if(err != OS_NO_ERR)
			{
				printf("ERROR setting vesa mode %d\n", err);
				return -1;
			}

		 	uint32_t sector = 0;
	    	uint8_t buffer[512];
		 	uint32_t index = 512;
		 	uint8_t  pixel[4] = {0};

		 	for(int m = 0; m < 1080; ++m)
		 	{
		 		for(int n = 0; n < 1920; ++n)
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
		}
	}

	vesa_clear_screen();

 	vesa_drawchar('A', 0, 0, 0x003498db, 0x00000000);
 	vesa_drawchar('l', 8, 0, 0x003498db, 0x00000000);
 	vesa_drawchar('e', 16, 0, 0x003498db, 0x00000000);
 	vesa_drawchar('x', 24, 0, 0x003498db, 0x00000000);
 	vesa_drawchar('y', 32, 0, 0x003498db, 0x00000000);

	return 0;
}