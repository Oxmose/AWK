/*******************************************************************************
 *
 * File: iota.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 08/01/2018
 *
 * Version: 1.0
 *
 * itoa function. To be used with stdlib.h header.
 *
 ******************************************************************************/

#include "../stddef.h" /* Generic int types */

/* Header include */
#include "../stdlib.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

 void itoa(int32_t i, char* buf, uint32_t base)
 {
     /* If base is unknown just return */
     if (base > 16)
     {
         return;
     }

     /* Check sign */
     if (base == 10 && i < 0)
     {
        *buf++ = '-';
        i *= -1;
     }

     /* To the job */
     uitoa(i, buf, base);
 }
