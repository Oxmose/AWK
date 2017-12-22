/*******************************************************************************
 *
 * File: driver_manager.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 22/12/2017
 *
 * Version: 1.0
 *
 * Kernel driver manager, load each driver registered in its table.
 ******************************************************************************/

#ifndef __DRIVER_MANAGER_H_
#define __DRIVER_MANAGER_H_

#include "../lib/stddef.h"  /* OS_RETURN_E */
#include "../lib/stdint.h"  /* Generic int types */

/****************************
 * CONSTANTS
 ***************************/
#define DRIVER_MAX_NAME_LENGTH 32

/****************************
 * STRUCTURES 
 ***************************/
/* Driver package information struct */
typedef struct driver_info
{
    char             name[DRIVER_MAX_NAME_LENGTH];
    OS_RETURN_E      (*init_driver)(void);

    struct driver_info *next;
} driver_info_t;

/****************************
 * FUNCTIONS 
 ***************************/
OS_RETURN_E init_driver_manager(void);

OS_RETURN_E register_driver(OS_RETURN_E (*init_func)(void), const char *name);

/* Must be called with interrupts disabled! */
OS_RETURN_E load_drivers(void);


#endif /* __DRIVER_MANAGER_H_ */