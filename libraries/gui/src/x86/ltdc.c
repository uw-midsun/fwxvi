/************************************************************************************************
 * @file   ltdc.c
 *
 * @brief  Source file for the LTDC driver on x86 (Local PC Simulation)
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "ltdc.h"

/* Intra-component Headers */
#include <SDL2/SDL.h>

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t *framebuffer;
    uint32_t *clut;
} LtdcConfig;

static LtdcConfig ltdc_config = {
    .width = 480,
    .height = 272, 
    .framebuffer = NULL,
    .clut = NULL
};

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void ltdc_init(void) {
    
}
 
void ltdc_set_framebuffer(uint8_t *buffer) {
    ltdc_config.framebuffer = buffer;
}

void ltdc_load_clut(uint32_t *table) {
    ltdc_config.clut = table;
}

void ltdc_draw(void) {

}