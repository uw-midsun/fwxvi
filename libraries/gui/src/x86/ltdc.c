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

#define SCREEN_WIDTH    480
#define SCREEN_HEIGHT   272

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static uint8_t *framebuffer = NULL;
static uint32_t *clut = NULL;

void ltdc_init_ltdc(void) {
    return 0;
}
 
void ltdc_set_framebuffer(uint8_t *buffer);
void ltdc_load_clut(void);
void ltdc_draw(void);