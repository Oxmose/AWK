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

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define DRIVER_MAX_NAME_LENGTH 32

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* Driver package information struct */
typedef struct driver_info
{
    char             name[DRIVER_MAX_NAME_LENGTH];

    /* Init function
     * !!! MUST NOT ENABLE INTERRUPT !!!
     */
    OS_RETURN_E      (*init_driver)(void);

    struct driver_info *next;
} driver_info_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Initialize the driver manager structures.
 *
 * @returns The function will return an error if the structures cannot be
 * initialized or OS_NO_ERR in case of success.
 */
OS_RETURN_E init_driver_manager(void);

/* Register a new driver to be loaded on loacd_drivers call. The init function
 * must be the function that initialize the driver and must not change interrupt
 * state (enable or disable). The name of the drivers is simply used for output
 * purpose.
 *
 * @param init_func The entry function to initialize the driver.
 * @param name The name of the driver, must be a null ternminated string of
 * DRIVER_MAX_NAME_LENGTH maximum length.
 *
 * @returns The function will return an error if the structures cannot be
 * initialized or OS_NO_ERR in case of success.
 */
OS_RETURN_E register_driver(OS_RETURN_E (*init_func)(void), const char *name);

/* Load each drivers registered with the function register_driver. Then each
 * driver will be removed from the table to avoid reloading them again.
 *
 * !!! MUST be called while interrupts are disabled !!!
 *
 * @returns The function will return an error if one driver has failed to be
 * loaded. OS_NO_ERR is returned in case of success.
 */
OS_RETURN_E load_drivers(void);

#endif /* __DRIVER_MANAGER_H_ */