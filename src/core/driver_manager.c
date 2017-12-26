/*******************************************************************************
 *
 * File: driver_manager.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 22/12/2017
 *
 * Version: 1.0
 *
 * Kernel driver manager, load each driver registered in its table.
 ******************************************************************************/

#include "../lib/stddef.h"  /* OS_RETURN_E */
#include "../lib/stdint.h"  /* Generic int types */
#include "../lib/malloc.h"  /* malloc, free */
#include "../lib/string.h"  /* strncpy */
#include "../sync/lock.h"   /* spinlocks */
#include "panic.h"		    /* kernel_panic */
#include "kernel_output.h"  /* kernel_success, kernel_error */

#include "../debug.h"      /* kernel_serial_debug */

/* Header file */
#include "driver_manager.h"

/* Drivers table */
static driver_info_t *drivers;
static driver_info_t *drivers_list_head;
static lock_t drivers_lock;

/****************************
 * FUNCTIONS 
 ***************************/
OS_RETURN_E init_driver_manager(void)
{
	spinlock_init(&drivers_lock);
	drivers           = NULL;
	drivers_list_head = NULL;

	return OS_NO_ERR;
}


OS_RETURN_E register_driver(OS_RETURN_E (*init_func)(void), const char *name)
{
	if(init_func == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	/* Alloc memory */
	driver_info_t *new_driver = malloc(sizeof(driver_info_t));
	if(new_driver == NULL)
	{
		return OS_ERR_MALLOC;
	}

	/* Init the structure */
	memset(new_driver, 0, sizeof(driver_info_t));

	/* Set the structure */
	strncpy(new_driver->name, name, DRIVER_MAX_NAME_LENGTH);
	new_driver->init_driver = init_func;

	#ifdef DEBUG_DRIVER_MANAGER
    kernel_serial_debug("Adding new driver to load: %s\n", name);
    #endif

	spinlock_lock(&drivers_lock);

	/* Place the driver in the list */
	if(drivers == NULL)
	{
		drivers = new_driver;
		drivers_list_head = new_driver;
	}
	else
	{
		drivers->next = new_driver;
		drivers = new_driver;
	}

	spinlock_unlock(&drivers_lock);

	return OS_NO_ERR;
}

OS_RETURN_E load_drivers(void)
{
	spinlock_lock(&drivers_lock);

	driver_info_t *cursor = drivers_list_head;
	driver_info_t *save;
	OS_RETURN_E err;

	while(cursor != NULL)
	{
		save = cursor;
		cursor = cursor->next;

		/* Call init function */
		err = save->init_driver();
		if(err == OS_NO_ERR)
		{
			kernel_success("%s Initialized\n", save->name);
		}
		else
		{
			kernel_error("%s Initialization error [%d]\n", save->name, err);
			kernel_panic();
		}

		/* Remove the driver from the list */
		free(save);
	}

	drivers = NULL;
	drivers_list_head = NULL;

	spinlock_unlock(&drivers_lock);

	return err;
}