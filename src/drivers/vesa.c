/*******************************************************************************
 *
 * File: vesa.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/01/2018
 *
 * Version: 1.0
 *
 * VESA VBE 2 graphic drivers
 ******************************************************************************/

#include "../memory/heap.h"    /* kmalloc, kfree */
#include "../lib/stdint.h"     /* Generic int types */
#include "../lib/stddef.h"     /* OS_RETURN_E */
#include "../lib/string.h"     /* memmove, memset */
#include "../bios/bios_call.h" /* regs_t, bios_call */
#include "../fonts/uni_vga.c"  /* __font_bitmap__ */
#include "vga_text.h"          /* vga_get_framebuffer */
#include "serial.h"            /* serial_write */
#include "graphic.h"           /* structures */

#include "../debug.h"      /* kernel_serial_debug */

/* Header file */
#include "vesa.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Screen settings */
#define MAX_SUPPORTED_HEIGHT 1000
#define MAX_SUPPORTED_WIDTH  1920
#define MAX_SUPPORTED_BPP    32

/* VESA data structures */
extern vbe_info_structure_t      vbe_info_base;
extern vbe_mode_info_structure_t vbe_mode_info_base;

/* VESA modes */
static vesa_mode_t* saved_modes;
static vesa_mode_t* current_mode;
static uint16_t     mode_count     = 0;
static uint8_t      vesa_supported = 0;

/* VESA console settigns, used in tty mode */
static cursor_t      screen_cursor;
static cursor_t      last_printed_cursor;
static colorscheme_t screen_scheme;

static uint32_t*    last_columns;

static const uint32_t vga_color_table[16] = {
    0x00000000,
    0x000000AA,
    0x0000aa00,
    0x0000aaaa,
    0x00aa0000,
    0x00aa00aa,
    0x00aa5500,
    0x00aaaaaa,
    0x00555555,
    0x005555ff,
    0x0055ff55,
    0x0055ffff,
    0x00ff5555,
    0x00ff55ff,
    0x00ffff55,
    0x00ffffff
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Process the character in parameters.
 *
 * @param character The character to process.
 */
static void vesa_process_char(const char character)
{
    uint32_t i;
    uint32_t j;

    int32_t diff;
    int8_t tab_width = 4;

    /* Write on serial */
    serial_write(COM1, character);

    /* If character is a normal ASCII character */
    if(character > 31 && character < 127)
    {
        /* Manage end of line cursor position */
        if(screen_cursor.x + font_width >= current_mode->width)
        {
            /* remove cursor */
            for(i = screen_cursor.x; i < current_mode->width; ++i)
            {
                for(j = screen_cursor.y; j < screen_cursor.y + font_height; ++j)
                {
                    vesa_draw_pixel(i, j,
                                   (screen_scheme.background & 0x00ff0000) >> 16,
                                   (screen_scheme.background & 0x0000ff00) >> 8,
                                   (screen_scheme.background & 0x000000ff));
                }
            }
            vesa_put_cursor_at(screen_cursor.y + font_height, 0);
            last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
        }

        /* Manage end of screen cursor position */
        if(screen_cursor.y >= current_mode->height - font_height)
        {
            vesa_scroll(SCROLL_DOWN, 1);
        }


        /* Display character */
        vesa_drawchar(character, screen_cursor.x, screen_cursor.y,
                      screen_scheme.foreground, screen_scheme.background);

        /* Update cursor position */
        vesa_put_cursor_at(screen_cursor.y, screen_cursor.x + font_width);

        /* Manage end of line cursor position */
        if(screen_cursor.x + font_width >= current_mode->width)
        {
            /* remove cursor */
            for(i = screen_cursor.x; i < current_mode->width; ++i)
            {
                for(j = screen_cursor.y; j < screen_cursor.y + font_height; ++j)
                {
                    vesa_draw_pixel(i, j,
                                   (screen_scheme.background & 0x00ff0000) >> 16,
                                   (screen_scheme.background & 0x0000ff00) >> 8,
                                   (screen_scheme.background & 0x000000ff));
                }
            }
            vesa_put_cursor_at(screen_cursor.y + font_height, 0);
        }
        last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
    }
    else
    {
        /* Manage special ACSII characters*/
        switch(character)
        {
            /* Backspace */
            case '\b':
                if(last_printed_cursor.y == screen_cursor.y)
                {
                    if(screen_cursor.x > last_printed_cursor.x)
                    {
                        vesa_drawchar(' ', screen_cursor.x, screen_cursor.y,
                                      screen_scheme.foreground,
                                      screen_scheme.background);
                        vesa_drawchar(' ', screen_cursor.x - font_width, screen_cursor.y,
                                      screen_scheme.foreground,
                                      screen_scheme.background);
                        vesa_put_cursor_at(screen_cursor.y,
                                           screen_cursor.x - font_width);
                        last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                    }
                }
                else if(last_printed_cursor.y < screen_cursor.y)
                {
                    if(screen_cursor.x > 0)
                    {
                        vesa_drawchar(' ', screen_cursor.x, screen_cursor.y,
                                      screen_scheme.foreground,
                                      screen_scheme.background);
                        vesa_drawchar(' ', screen_cursor.x - font_width, screen_cursor.y,
                                      screen_scheme.foreground,
                                      screen_scheme.background);
                        vesa_put_cursor_at(screen_cursor.y,
                                           screen_cursor.x - font_width);
                        last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;

                    }
                    else
                    {
                        vesa_drawchar(' ', screen_cursor.x, screen_cursor.y,
                                      screen_scheme.foreground,
                                      screen_scheme.background);
                        vesa_drawchar(' ', last_columns[(screen_cursor.y / font_height) - 1], screen_cursor.y - font_height,
                                      screen_scheme.foreground,
                                      screen_scheme.background);
                        vesa_put_cursor_at(screen_cursor.y - font_height,
                                           last_columns[(screen_cursor.y / font_height) - 1]);
                    }
                }
                break;
            /* Tab */
            case '\t':
                diff = current_mode->width -
                       (screen_cursor.x + tab_width * font_width);

                if(diff < 0)
                {
                    tab_width = tab_width + (diff / font_width);
                }
                while(tab_width-- > 0)
                {
                    vesa_process_char(' ');
                }
                last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                break;
            /* Line feed */
            case '\n':

                /* remove cursor */
                for(i = screen_cursor.x; i < current_mode->width; ++i)
                {
                    for(j = screen_cursor.y; j < screen_cursor.y + font_height; ++j)
                    {
                        vesa_draw_pixel(i, j,
                                       (screen_scheme.background & 0x00ff0000) >> 16,
                                       (screen_scheme.background & 0x0000ff00) >> 8,
                                       (screen_scheme.background & 0x000000ff));
                    }
                }
                last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                if(screen_cursor.y + font_height < current_mode->height - font_height)
                {
                    /* remove cursor */
                    for(i = screen_cursor.x; i < current_mode->width &&  i < screen_cursor.x + font_width; ++i)
                    {
                        for(j = screen_cursor.y; j < screen_cursor.y + font_height; ++j)
                        {
                            vesa_draw_pixel(i, j,
                                           (screen_scheme.background & 0x00ff0000) >> 16,
                                           (screen_scheme.background & 0x0000ff00) >> 8,
                                           (screen_scheme.background & 0x000000ff));
                        }
                    }
                    vesa_put_cursor_at(screen_cursor.y + font_height, 0);
                    last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                }
                else
                {
                    vesa_scroll(SCROLL_DOWN, 1);
                }
                break;
            /* Clear screen */
            case '\f':
                vesa_clear_screen();
                break;
            /* Line return */
            case '\r':
                vesa_put_cursor_at(screen_cursor.y, 0);
                last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                break;
            /* Undefined */
            default:
                break;
        }
    }
}

OS_RETURN_E init_vesa(void)
{
    bios_int_regs_t regs;
    uint32_t        i;
    uint16_t*       modes;
    vesa_mode_t*    new_mode;

    /* Init data */
    mode_count     = 0;
    vesa_supported = 0;
    current_mode   = NULL;
    saved_modes    = NULL;

    /* Console mode init */
    screen_cursor.x = 0;
    screen_cursor.y = 0;
    screen_scheme.foreground = 0x00FFFFFF;
    screen_scheme.background = 0x00000000;
    screen_scheme.vga_color  = 0;

    /* Init structure */
    strncpy(vbe_info_base.signature, "VBE2", 4);

     /* Init the registers for the bios call */
    regs.ax = BIOS_CALL_GET_VESA_INFO;
    regs.es = 0;
    regs.di = (uint16_t)(uint32_t)(&vbe_info_base);

    /* Issue call */
    bios_int(BIOS_INTERRUPT_VESA, &regs);


    /* Check call result */
    if(regs.ax != 0x004F || strncmp(vbe_info_base.signature, "VESA", 4) != 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    /* Get modes */
    modes = (uint16_t*)vbe_info_base.video_modes;
    for (i = 0 ; mode_count < MAX_VESA_MODE_COUNT && modes[i] != 0xFFFF ; ++i)
    {
        /* Prepare registers for mode query call */
        regs.ax = BIOS_CALL_GET_VESA_MODE;
        regs.cx = modes[i];
        regs.es = 0;
        regs.di = (uint16_t)(uint32_t)(&vbe_mode_info_base);

        bios_int(BIOS_INTERRUPT_VESA, &regs);

        /* Check call result */
        if(regs.ax != 0x004F)
        {
            continue;
        }

        /* The driver only support linear frame buffer management */
        if ((vbe_mode_info_base.attributes & VESA_FLAG_LINEAR_FB) !=
            VESA_FLAG_LINEAR_FB)
        {
            continue;
        }

        /* Check if this is a packed pixel or direct color mode */
        if (vbe_mode_info_base.memory_model != 4 &&
            vbe_mode_info_base.memory_model != 6 )
        {
            continue;
        }

        new_mode = kmalloc(sizeof(vesa_mode_t));
        if(new_mode == NULL)
        {
            return OS_ERR_MALLOC;
        }

        /* The mode is compatible, save it */
        new_mode->width       = vbe_mode_info_base.width;
        new_mode->height      = vbe_mode_info_base.height;
        new_mode->bpp         = vbe_mode_info_base.bpp;
        new_mode->mode_id     = modes[i];
        new_mode->framebuffer = vbe_mode_info_base.framebuffer;

        /* Save mode in list */
        new_mode->next = saved_modes;
        saved_modes = new_mode;

        ++mode_count;
    }

    if(mode_count > 0)
    {
        vesa_supported = 1;
    }

    return OS_NO_ERR;
}

OS_RETURN_E text_vga_to_vesa(void)
{
    OS_RETURN_E       err;
    uint32_t          i;
    uint32_t          j;
    vesa_mode_info_t  selected_mode;
    vesa_mode_t*      cursor;
    uint16_t*         vga_fb;
    cursor_t          vga_cursor;
    uint16_t          temp_buffer[SCREEN_LINE_SIZE * SCREEN_COL_SIZE];
    colorscheme_t     new_colorscheme;
    colorscheme_t     old_colorscheme = screen_scheme;

    /* Save VGA content */
    vga_save_cursor(&vga_cursor);
    vga_fb = vga_get_framebuffer(0, 0);
    memcpy(temp_buffer, vga_fb,
           sizeof(uint16_t) * SCREEN_LINE_SIZE * SCREEN_COL_SIZE);

    /* Set VESA mode */
    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    if(mode_count == 0)
    {
        return OS_NO_ERR;
    }

    selected_mode.width = 0;
    selected_mode.height = 0;
    selected_mode.bpp = 0;


    cursor = saved_modes;
    while(cursor != NULL)
    {
        if(cursor->width > MAX_SUPPORTED_WIDTH ||
           cursor->height > MAX_SUPPORTED_HEIGHT ||
           cursor->bpp > MAX_SUPPORTED_BPP)
        {
            cursor = cursor->next;
            continue;
        }

        if(cursor->width >= selected_mode.width &&
           cursor->height >= selected_mode.height &&
           cursor->bpp  >= selected_mode.bpp)
        {
            selected_mode.width = cursor->width;
            selected_mode.height = cursor->height;
            selected_mode.bpp = cursor->bpp;
            selected_mode.mode_id = cursor->mode_id;
        }
        cursor = cursor->next;
    }
    #ifdef DEBUG_VESA
    kernel_serial_debug("SELECTED VESA mode %dx%d %dbits\n",
                                                  selected_mode.width,
                                                  selected_mode.height,
                                                  selected_mode.bpp);
    #endif

    err = set_vesa_mode(selected_mode);

    if(err != OS_NO_ERR)
    {
        return err;
    }

    vesa_clear_screen();

    vga_fb = temp_buffer;

    /* Browse the framebuffer to get the cahracters and the colorscheme */
    for(i = 0; i < SCREEN_LINE_SIZE; ++i)
    {
        for(j = 0; j < SCREEN_COL_SIZE; ++j)
        {
            if(vga_cursor.y < i)
            {
                break;
            }
            else if(vga_cursor.y == i && vga_cursor.x == j)
            {
                break;
            }

            /* Get data character */
            char character = (*vga_fb) & 0x00FF;
            new_colorscheme.foreground =
                vga_color_table[((*vga_fb) & 0x0F00) >> 8];
            new_colorscheme.background =
                vga_color_table[((*vga_fb) & 0xF000) >> 12];

            /* Print char */
            vesa_set_color_scheme(new_colorscheme);
            vesa_process_char(character);

            ++vga_fb;
        }
        if(vga_cursor.y == i)
        {
            break;
        }
        vesa_process_char('\n');
    }

    /* Restore previous screen scheme */
    screen_scheme = old_colorscheme;
    return OS_NO_ERR;
}

uint16_t get_vesa_mode_count(void)
{
    return mode_count;
}

OS_RETURN_E get_vesa_modes(vesa_mode_info_t* buffer, const uint32_t size)
{
    uint32_t i;
    vesa_mode_t* cursor;

    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    if(mode_count == 0)
    {
        return OS_NO_ERR;
    }

    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    i = 0;
    cursor = saved_modes;
    while(cursor != NULL && i < size)
    {
        buffer[i].width       = cursor->width;
        buffer[i].height      = cursor->height;
        buffer[i].bpp         = cursor->bpp;
        buffer[i].mode_id     = cursor->mode_id;
        cursor = cursor->next;
        ++i;
    }

    return OS_NO_ERR;
}

OS_RETURN_E set_vesa_mode(const vesa_mode_info_t mode)
{
    bios_int_regs_t regs;
    uint32_t        last_columns_size;
    vesa_mode_t*    cursor;

    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    /* Search for the mode in the saved modes */
    cursor = saved_modes;
    while(cursor != NULL)
    {
        if(cursor->mode_id == mode.mode_id &&
           cursor->width == mode.width     &&
           cursor->height == mode.height   &&
           cursor->bpp == mode.bpp)
        {
            break;
        }
        cursor = cursor->next;
    }

    /* The mode was not found */
    if(cursor == NULL)
    {
        return OS_ERR_VESA_MODE_NOT_SUPPORTED;
    }

    last_columns_size = sizeof(uint32_t) * current_mode->height / font_height;

    if(last_columns != NULL)
    {
        kfree(last_columns);
    }

    last_columns = kmalloc(last_columns_size);
    if(last_columns == NULL)
    {
        return OS_ERR_MALLOC;
    }

    /* Set the last collumn array */
    memset(last_columns, 0, last_columns_size);

    /* Set the VESA mode */
    regs.ax = BIOS_CALL_SET_VESA_MODE;
    regs.bx = cursor->mode_id | VESA_FLAG_LFB_ENABLE;
    bios_int(BIOS_INTERRUPT_VESA, &regs);

    /* Check call result */
    if(regs.ax != 0x004F)
    {
        return OS_ERR_VESA_MODE_NOT_SUPPORTED;
    }

    current_mode = cursor;

    /* Tell generic driver we loaded a VESA mode */
    set_selected_driver(VESA_DRIVER);

    return OS_NO_ERR;
}

OS_RETURN_E vesa_draw_pixel(const uint16_t x, const uint16_t y,
                            const uint8_t red, const uint8_t green,
                            const uint8_t blue)
{
    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    if(current_mode == NULL)
    {
        return OS_ERR_VESA_NOT_INIT;
    }

    if(x > current_mode->width || y > current_mode->height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Get framebuffer address */
    uint32_t* addr = ((uint32_t*)current_mode->framebuffer) +
                     (current_mode->width * y) + x;

    uint8_t pixel[4] = {blue, green, red, 0x00};

    *addr = *((uint32_t*)pixel);

    return OS_NO_ERR;
}

void vesa_drawchar(const unsigned char charracter,
                   const uint32_t x, const uint32_t y,
                   const uint32_t fgcolor, const uint32_t bgcolor)
{
    uint32_t cx;
    uint32_t cy;

    uint32_t mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

    unsigned char *glyph = __font_bitmap__ + (charracter - 31) * 16;

    uint8_t pixel[4] = {0};

    for(cy = 0; cy < 16; ++cy)
    {
        for(cx = 0; cx < 8; ++cx)
        {
            *((uint32_t*)pixel) = glyph[cy] & mask[cx] ? fgcolor : bgcolor;

            vesa_draw_pixel(x + (7 - cx ), y + cy,
                            pixel[2], pixel[1], pixel[0]);
        }
    }
}

void vesa_clear_screen(void)
{
    memset((void*)current_mode->framebuffer, 0,
           current_mode->width *
           current_mode->height *
           (current_mode->bpp / 8));
    memset(last_columns, 0, sizeof(uint32_t) * current_mode->height / font_width);
    vesa_put_cursor_at(0, 0);
}

OS_RETURN_E vesa_put_cursor_at(const uint32_t line, const uint32_t column)
{
    uint8_t i;

    screen_cursor.x = column;
    screen_cursor.y = line;

    if(column + 2 < current_mode->width)
    {
        for(i = 0; i < 16; ++i)
        {
            vesa_draw_pixel(column, line + i, 0xFF, 0xFF, 0xFF);
            vesa_draw_pixel(column + 1, line + i, 0xFF, 0xFF, 0xFF);
        }
    }

    return OS_NO_ERR;
}

OS_RETURN_E vesa_save_cursor(cursor_t* buffer)
{
    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Save cursor attributes */
    buffer->x = screen_cursor.x;
    buffer->y = screen_cursor.y;

    return OS_NO_ERR;
}

OS_RETURN_E vesa_restore_cursor(const cursor_t buffer)
{
    if(buffer.x >= current_mode->width || buffer.y >= current_mode->height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Restore cursor attributes */
    vesa_put_cursor_at(buffer.y, buffer.x);

    return OS_NO_ERR;
}

/* Scroll in the desired direction of lines_count lines.
 *
 * @param direction The direction to whoch the console should be scrolled.
 * @param lines_count The number of lines to scroll.
 */
void vesa_scroll(const SCROLL_DIRECTION_E direction,
                 const uint32_t lines_count)
{
    uint8_t to_scroll;
    uint8_t i;
    uint8_t j;
    int32_t q;
    int32_t m;

    to_scroll = lines_count;

    q = current_mode->height / font_height;
    m = current_mode->height % (q * font_height);

    /* Select scroll direction */
    if(direction == SCROLL_DOWN)
    {
        /* For each line scroll we want */
        for(j = 0; j < to_scroll; ++j)
        {
            /* Copy all the lines to the above one */
            for(i = 0; i < q; ++i)
            {
                memmove(((uint32_t*)current_mode->framebuffer) + i * (font_height * current_mode->width),
                        ((uint32_t*)current_mode->framebuffer) + (i + 1) * (font_height * current_mode->width),
                        (current_mode->bpp / 8) * font_height * current_mode->width);
                last_columns[i] = last_columns[i + 1];
            }
        }
    }

    /* Replace cursor */
    vesa_put_cursor_at(current_mode->height - m - font_height, 0);
    last_columns[(screen_cursor.y / font_height)] = 0;

    if(to_scroll <= last_printed_cursor.y)
    {
        last_printed_cursor.y -= to_scroll * font_height;
    }
    else
    {
        last_printed_cursor.x = 0;
        last_printed_cursor.y = 0;
    }
}

void vesa_set_color_scheme(const colorscheme_t color_scheme)
{
    screen_scheme.foreground = color_scheme.foreground;
    screen_scheme.background = color_scheme.background;
}

OS_RETURN_E vesa_save_color_scheme(colorscheme_t* buffer)
{
    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Save color scheme into buffer */
    buffer->foreground = screen_scheme.foreground;
    buffer->background = screen_scheme.background;

    return OS_NO_ERR;
}

void vesa_put_string(const char* string)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < strlen(string); ++i)
    {
        vesa_process_char(string[i]);
    }
    last_printed_cursor = screen_cursor;
}

void vesa_put_char(const char charactrer)
{
    vesa_process_char(charactrer);
    last_printed_cursor = screen_cursor;
}

void vesa_console_write_keyboard(const char* string, const uint32_t size)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < size; ++i)
    {
        vesa_process_char(string[i]);
    }
}
