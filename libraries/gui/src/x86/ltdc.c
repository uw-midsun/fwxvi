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

    SDL_Window *window;
    SDL_Renderer *renderer;
} LtdcConfig;

static LtdcConfig ltdc_config = {
    .width = 480,
    .height = 272, 
    .framebuffer = NULL,
    .clut = NULL,

    .window = NULL,
    .renderer = NULL
};

void ltdc_init(void) {
    /* Set up SDL2 Video subsystem (no audio, timers, etc. required for now) */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());

        return;
    }

    /* Create a window to display graphics on */
    ltdc_config.window = SDL_CreateWindow(
        "GUI Simulation",           // Window Title
        SDL_WINDOWPOS_CENTERED,     // Horizontally Centred
        SDL_WINDOWPOS_CENTERED,     // Vertically Centred
        ltdc_config.width,          // Window Width
        ltdc_config.height,         // Window Height
        0                           // No flags
    );

    if (!ltdc_config.window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();

        return;
    }

    /* Create a renderer for the window (to actually draw graphics on it) */
    ltdc_config.renderer = SDL_CreateRenderer(
        ltdc_config.window,         // Created Window
        -1,                         // Rendering Driver (picks first available)
        SDL_RENDERER_ACCELERATED    // Flag for Hardware Acceleration
    );

    if (!ltdc_config.renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        
        return;
    }
}
 
void ltdc_set_framebuffer(uint8_t *buffer) {
    ltdc_config.framebuffer = buffer;
}

void ltdc_load_clut(uint32_t *table) {
    ltdc_config.clut = table;
}

void ltdc_draw(void) {
    // TODO: Implement drawing using framebuffer and CLUT!
}