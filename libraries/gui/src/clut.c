/************************************************************************************************
 * @file   clut.c
 *
 * @brief  Source file for the CLUT
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "clut.h"

uint8_t const* COLOURS[256][3] = 
{
    [COLOUR_BLACK] = {0,0,0},
    [COLOUR_WHITE] = {255,255,255},
    [COLOUR_RED] = {255,0,0},
    [COLOUR_GREEN] = {0,255,0},
    [COLOUR_BLUE] = {0,0,255}
};

uint8_t* get_colour(uint8_t* rgb_var, ClutColours colour) {
    return COLOURS[colour];
}