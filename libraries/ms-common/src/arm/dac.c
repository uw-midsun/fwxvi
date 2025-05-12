/************************************************************************************************
 * @file    dac.c
 *
 * @brief   DAC Library Source file
 *
 * @date    2025-05-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include "stm32l4xx_hal_conf.h"

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "ms_semaphore.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_dac.h"

/* Intra-component Headers */
#include "dac.h"

DAC_HandleTypeDef s_dac_handle;

StatusCode dac_init(const DacConfig *config) {
  // Step 1: Enable the clock for DAC1 peripheral so its registers are accessible
  __HAL_RCC_DAC1_CLK_ENABLE();

  // Step 2: Point the HAL handle to DAC1 (this tells HAL which DAC to work with)
  s_dac_handle.Instance = DAC1;

  // Step 3: Initialize the DAC peripheral using HAL
  if (HAL_DAC_Init(&s_dac_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR ;
  }

  // Step 4: Configure shared settings for both channels
  DAC_ChannelConfTypeDef dac_channel_config = {0};  // Zero-initialize the struct
  dac_channel_config.DAC_Trigger = DAC_TRIGGER_NONE;              // Manual trigger: update only when we call SetValue
  dac_channel_config.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;  // Enable buffer to drive analog output cleanly

  // Step 5: Configure and start Channel 1
  if (HAL_DAC_ConfigChannel(&s_dac_handle, &dac_channel_config, MS_DAC_CHANNEL_1) != HAL_OK ||
      HAL_DAC_Start(&s_dac_handle, MS_DAC_CHANNEL_1) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  // Step 6: Configure and start Channel 2 (optional — only if needed)
  if (HAL_DAC_ConfigChannel(&s_dac_handle, &dac_channel_config, MS_DAC_CHANNEL_2) != HAL_OK ||
      HAL_DAC_Start(&s_dac_handle, MS_DAC_CHANNEL_2) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  // Step 7: Return success if everything went well
  return STATUS_CODE_OK;
}
