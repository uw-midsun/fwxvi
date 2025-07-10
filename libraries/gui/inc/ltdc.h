#pragma once

/************************************************************************************************
 * @file   ltdc.h
 *
 * @brief  Header file for the LTDC driver
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#define LTDC
#include <stm32l4xx_hal_ltdc.h>

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t *framebuffer;
    uint32_t *clut;

    LTDC_LayerCfgTypeDef layer_cfg;   
} LtdcConfig;

void init_ltdc(void);
void set_framebuffer(uint8_t *buffer);
void load_clut(void);

/** @} */
