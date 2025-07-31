#pragma once

/************************************************************************************************
 * @file   clut.h
 *
 * @brief  Header file for the CLUT
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef enum {
    COLOUR_BLACK,
    COLOUR_WHITE,
    COLOUR_RED,
    COLOUR_GREEN,
    COLOUR_BLUE
} ClutColours;

uint8_t* get_colour(uint8_t* rgb_var, ClutColours colour);

/** @} */
