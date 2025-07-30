#pragma once

/************************************************************************************************
 * @file   ltdc.h
 *
 * @brief  Header file for the platform-agnostic LTDC API
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

void ltdc_init(void);
void ltdc_set_framebuffer(uint8_t *buffer);
void ltdc_load_clut(uint32_t *table);
void ltdc_draw(void);

/** @} */
