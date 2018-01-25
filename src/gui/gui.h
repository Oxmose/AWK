/*******************************************************************************
 *
 * File: gui.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/01/2018
 *
 * Version: 1.0
 *
 * GUI manager.
 ******************************************************************************/

#ifndef __GUI_H__
#define __GUI_H__

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
* STRUCTURES
*******************************************************************************/

/*******************************************************************************
* FUNCTIONS
*******************************************************************************/

OS_RETURN_E start_gui(void);

OS_RETURN_E gui_fill_rect(const uint32_t* ptr,
                          const uint32_t x, const uint32_t y,
                          const uint32_t width, const uint32_t height,
                          const uint32_t width_gap);

OS_RETURN_E gui_draw_rect(const uint32_t x, const uint32_t y,
                          const uint32_t width, const uint32_t height,
                          const uint8_t alpha, const uint8_t red,
                          const uint8_t green, const uint8_t blue);

OS_RETURN_E gui_draw_pixel(const uint32_t x, const uint32_t y,
                           const uint8_t alpha, const uint8_t red,
                           const uint8_t green, const uint8_t blue);

OS_RETURN_E gui_drawchar(const unsigned char charracter,
                        const uint32_t x, const uint32_t y,
                        const uint32_t fgcolor, const uint32_t bgcolor);

#endif /* __GUI_H__ */
