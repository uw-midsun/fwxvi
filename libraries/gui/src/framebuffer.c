/************************************************************************************************
 * @file   framebuffer.c
 *
 * @brief  Source file for the framebuffer
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "framebuffer.h"

uint8_t frame_buffer_array[SCREEN_WIDTH * SCREEN_LENGTH] = {0};

void init_buffer(FrameBuffer* buffer) {
    buffer->screen_width = SCREEN_WIDTH;
    buffer->screen_length = SCREEN_LENGTH;
    buffer->pixel_data = &frame_buffer_array;
}

void write_buffer(FrameBuffer* buffer, int row, int col, ClutColours colour) {
    buffer->pixel_data[row*SCREEN_WIDTH + col] = colour;
}

uint8_t read_buffer(FrameBuffer* buffer, int row, int col) {
    return buffer->pixel_data[row*SCREEN_WIDTH + col];
}