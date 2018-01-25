/*******************************************************************************
 *
 * File: windoz.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/01/2018
 *
 * Version: 1.0
 *
 * GUI window manager.
 ******************************************************************************/

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
* STRUCTURES
******************************************************************************/

typedef struct window
{
    int32_t pos_x;
    int32_t pos_y;

    uint32_t width;
    uint32_t height;

    uint32_t* canvas;
} window_t;

/*******************************************************************************
* FUNCTIONS
******************************************************************************/
OS_RETURN_E init_window(window_t *window,
                        const int32_t x, const int32_t y,
                        const uint32_t width, const uint32_t height);

OS_RETURN_E draw_window(const window_t *window);

#endif /* __WINDOW_H__ */
