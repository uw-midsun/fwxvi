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
#include "font8x8.c"

/* Intra-component Headers */
#include "status.h"

/* Standard Library Headers */


StatusCode gui_init(GuiSettings *settings){
    if(!settings){
        return STATUS_CODE_INVALID_ARGS;
    }
    StatusCode framebuffer_status = framebuffer_init(&settings->framebuffer, settings->ltdc.width, settings->ltdc.height, settings->ltdc.framebuffer);
    if(framebuffer_status != STATUS_CODE_OK){
        LOG_DEBUG("Framebuffer_init error: %d\n", framebuffer_status);
        return framebuffer_status;
    }
    StatusCode ltdc_status = ltdc_init(&settings->ltdc); 
    if(ltdc_status != STATUS_CODE_OK){
        LOG_DEBUG("ltdc_init error: %d\n", ltdc_status);
        return ltdc_status;
    }

    return STATUS_CODE_OK;
}


void gui_draw_pixel(uint16_t x, uint16_t y, ColorIndex color_index){
    ltdc_set_pixel(x, y, color_index);
}


void gui_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorIndex color_index){
    for (uint16_t i=0;i<width;++i){
        for (uint16_t j=0; j<height;++j){
            ltdc_set_pixel(x+i, y+j, color_index);
        }
    }
}


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


StatusCode gui_render(void){
    return ltdc_draw();
}

void gui_progress_bar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t percentage, ColorIndex color_index_bg, ColorIndex color_index_fg){
    uint16_t filled_width = width * percentage / 100;
    for (uint16_t row = 0;row < height; ++row){
        gui_draw_line(x, y+row, x+filled_width-1, y+row, color_index_fg);
        gui_draw_line(x+filled_width, y+row, x+width-1, y+row, color_index_bg);
    }
}


void gui_display_char(uint16_t x, uint16_t y, char c, ColorIndex color_index){
    for (uint8_t row=0; row<8; ++row){
        uint8_t bits = font8x8[(uint8_t)c][row];
        for (uint8_t col= 0; col<8; ++col){
            if(bits & 1<<(7-col)){
                gui_draw_pixel(x+col, y+row, color_index);
            }
        }

    }
}

void gui_display_text(uint16_t x, uint16_t y, const char *text, ColorIndex color_index){
    uint16_t cursor_x = x;

    
    while(*text){
        gui_display_char(cursor_x, y, *text, color_index);
        text++;
        cursor_x+=8;
    }

}
