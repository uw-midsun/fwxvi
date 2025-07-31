#pragma once

/************************************************************************************************
 * @file   framebuffer.h
 *
 * @brief  Header file for the framebuffer
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "clut.h"

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

#define SCREEN_WIDTH 480
#define SCREEN_LENGTH 272

typedef struct {
    int screen_width;
    int screen_length;
    uint8_t* pixel_data;
} FrameBuffer;

void init_buffer(FrameBuffer* buffer);
void write_buffer(FrameBuffer* buffer, int row, int col, ClutColours colour);
uint8_t read_buffer(FrameBuffer* buffer, int row, int col);

/** @} */
