/************************************************************************************************
 * @file   bootloader_crc32.c
 *
 * @brief  Source file for the CRC32 calculations in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_crc.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "bootloader_crc32.h"

static CRC_HandleTypeDef s_crc_handle; 

BootloaderError boot_crc32_init(void) {
  __HAL_RCC_CRC_CLK_ENABLE(); 
  s_crc_handle.Instance = CRC;
  s_crc_handle.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;

  s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_32B;
  s_crc_handle.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
  s_crc_handle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  s_crc_handle.Init.InitValue = 0xFFFFFFFFU;

  s_crc_handle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  s_crc_handle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;

  if (HAL_CRC_Init(&s_crc_handle) != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  return BOOTLOADER_ERROR_NONE;
}

uint32_t boot_crc32_calculate(const uint32_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return 0U;
  }

  return HAL_CRC_Calculate(&s_crc_handle, (uint32_t *)buffer, buffer_len);
}

void boot_align_to_32bit_words(uint8_t *buffer, size_t *buffer_len) {
  if (buffer == NULL || buffer_len == NULL) {
    return; 
  }

  size_t remainder = *buffer_len % ALLIGNMENT_SIZE; 
  if (remainder) {
    return; 
  }

  size_t padding = ALLIGNMENT_SIZE - remainder; 
  memset(buffer + *buffer_len, 0, padding); 
  *buffer_len += padding; 

  return; 
}
