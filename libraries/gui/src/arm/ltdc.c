/************************************************************************************************
 * @file   ltdc.c
 *
 * @brief  Source file for the LTDC driver on arm (STM32)
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "ltdc.h"

/* Intra-component Headers */
#define LTDC
#include <stm32l4xx_hal_ltdc.h>

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t *framebuffer;
    uint32_t *clut;

    LTDC_ColorTypeDef color;
    LTDC_InitTypeDef init;
    LTDC_LayerCfgTypeDef layer_cfg;   
} LtdcConfig;