#pragma once

/************************************************************************************************
 * @file   gui.c
 *
 * @brief  Source file for the GUI library
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "clut.h"
#include "framebuffer.h"
#include "ltdc.h"
#include "gui.h"
/* Intra-component Headers */
#include "status.h"

/* Standard Library Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */


/**
 * @brief   Initialize GUI subsystem (framebuffer, CLUT, LTDC)
 * @param   settings Pointer to GUI configuration
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_init(GuiSettings *settings){
    if(settings == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }
    StatusCode framebuffer_status = framebuffer_init(&settings->framebuffer, settings->ltdc.width, settings->ltdc.height, settings->ltdc.framebuffer);
    if(framebuffer_status != STATUS_CODE_OK){
        return framebuffer_status;
    }
    StatusCode ltdc_status = ltdc_init(&settings->ltdc); 
    if(ltdc_status != STATUS_CODE_OK){
        return ltdc_status;
    }

    return STATUS_CODE_OK;
}

/**
 * @brief   Draw a single pixel at (x, y)
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color_index CLUT color index
 */
void gui_draw_pixel(uint16_t x, uint16_t y, ColorIndex color_index){
    ltdc_set_pixel(x, y, color_index);
}

/**
 * @brief   Draw a filled rectangle
 * @param   x X coordinate of the top left corner of the rectangle
 * @param   y Y coordinate of the top left corner of the rectangle
 * @param   width Width in pixels
 * @param   height Height in pixels
 * @param   color_index CLUT color index
 */
void gui_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorIndex color_index){
    for (uint16_t i=0;i<width;++i){
        for (uint16_t j=0; j<height;++j){
            ltdc_set_pixel(x+i, y+j, color_index);
        }
    }
}

/**
 * @brief   Draw a line
 * @param   x0 X coordinate start location
 * @param   y0 Y coordinate start location
 * @param   x1 X coordinate end location
 * @param   y1 Y coordinate end location
 * @param   color_index CLUT color index
 */
void gui_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorIndex color_index){
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = (x0 < x1)? 1 : -1;
    int sy = (y0 < y1)? 1 : -1;
    int err = dx - dy;

    while(1){
        ltdc_set_pixel(x0, y0, color_index);

        if (x0 == x1 && y0 == y1){
            break;
        }

        int e2 = 2*err;
        if (e2>=-dy){
            err -= dy;
            x0 += sx;
        }
        if (e2<=dx){
            err += dx;
            y0 += sy;
        }
    }
}

/**
 * @brief   Update display with current framebuffer
 */
StatusCode gui_render(void){
    return ltdc_draw();
}

/** @} */
