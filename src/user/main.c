#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../core/scheduler.h"

#include "../drivers/vga_graphic.h"
#include "../drivers/ata.h"
#include "../drivers/keyboard.h"

#include "../sync/semaphore.h"

#include "../core/kernel_output.h"
#include "../cpu/bios_call.h"

extern void* launch_tests(void*);
extern int test_dyn_sched(void);

uint8_t image[320 * 200] = {0};
uint8_t image2[320 * 200] = {0};

semaphore_t load_sem;
semaphore_t disp_sem;

void* load_image(void* args)
{
	(void)args;
	uint32_t chunks = 125;
	uint32_t chunk_size = 512;

	uint32_t offset = 0;
	int8_t selector = 0;

	uint8_t *image_ptr;

	uint32_t max_offset = chunks * 3;


    ata_device_t dev;
    dev.port = PRIMARY_PORT;
    dev.type = MASTER;

	while(1)
	{
		if(selector == 1)
		{
			image_ptr = image2;
			selector = 0;
		}
		else
		{
			image_ptr = image;
			selector = 1;
		}

		for(uint32_t i = 0; i < chunks; ++i)
		{

			if(ata_read_sector(dev, offset + i, image_ptr + (i * chunk_size), chunk_size) != OS_NO_ERR)
			{
				kernel_serial_debug("Error loading image %d\n", selector);
				return NULL;
			}
		}
		kernel_serial_debug("Loaded image %d\n", selector);

		sem_post(&disp_sem);
		sem_post(&disp_sem);
		sem_pend(&load_sem);

		offset += chunks;
		offset %= max_offset;
	}
}

void* draw_logo(void *args)
{
	int8_t selector = 0;
	while(1)
	{

		uint8_t *image_ptr;

		if(selector == 1)
		{
			image_ptr = image2;
			selector = 0;
		}
		else
		{
			image_ptr = image;
			selector = 1;
		}

		sem_pend(&disp_sem);

	    if((int)args == 0)
	    {
	    	sem_post(&load_sem);

	        int limit = 320;
	        for(int j = 0; j < 200; ++j)
	        {


	            for(int i = 0; i < limit; ++i)
	            {
	                int8_t pixel = image_ptr[j * 320 + i];
	                int8_t r = pixel >> 5 & 0x7;
	                int8_t g = pixel >> 2 & 0x7;
	                int8_t b = pixel & 0x3;
	                draw_pixel(i, j, r, g, b);
	            }
	            if(j % 5 == 0)
	                limit -= 8;
	            sleep(1);
	        }

	    }

	    else if((int)args == 1)
	    {
	        int limit = 320;
	        for(int j = 199; j >= 0; --j)
	        {
	            for(int i = 0; i < limit; ++i)
	            {
	                int8_t pixel = image_ptr[j * 320 + 320 - limit + i];
	                int8_t r = pixel >> 5 & 0x7;
	                int8_t g = pixel >> 2 & 0x7;
	                int8_t b = pixel & 0x3;
	                draw_pixel(320 - limit + i, j, r, g, b);
	            }
	            if(j % 5 == 0)
	                limit -= 8;
	            sleep(1);
	        }

	    }
	    sleep(1000);

	}
    return NULL;
}



int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
    thread_t test_th;
    create_thread(&test_th, launch_tests, 64, "tests\0", NULL);

    wait_thread(test_th, NULL);

    printf("\nPress enter key to start demo");
    char t;
    getch(&t);

    init_vga();

    sem_init(&load_sem, 0);
    sem_init(&disp_sem, 0);

    create_thread(NULL, load_image, 32, "tests\0", (void*)0);
    create_thread(NULL, draw_logo, 32, "tests\0", (void*)0);
    create_thread(NULL, draw_logo, 32, "tests\0", (void*)1);


    return 0;
}