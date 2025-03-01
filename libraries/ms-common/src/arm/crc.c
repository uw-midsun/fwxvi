/************************************************************************************************
 * @file   crc.c
 *
 * @brief  CRC Library Source file
 *
 * @date   2025-02-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_crc.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "crc.h"

static CRC_HandleTypeDef s_crc_handle;

StatusCode crc_init(CRCLength crc_length) {
  __HAL_RCC_CRC_CLK_ENABLE();

  s_crc_handle.Instance = CRC;
  s_crc_handle.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;

  if (crc_length == CRC_LENGTH_8) {
    s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_8B;
    s_crc_handle.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    s_crc_handle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    s_crc_handle.Init.GeneratingPolynomial = 0x07U; /* Standard CRC-8 polynomial */
    s_crc_handle.Init.InitValue = 0xFFU;
  } else if (crc_length == CRC_LENGTH_16) {
    s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_16B;
    s_crc_handle.InputDataFormat = CRC_INPUTDATA_FORMAT_HALFWORDS;
    s_crc_handle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    s_crc_handle.Init.GeneratingPolynomial = 0x1021U; /* Standard CRC-16-CCITT polynomial */
    s_crc_handle.Init.InitValue = 0xFFFFU;
  } else if (crc_length == CRC_LENGTH_32) {
    s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_32B;
    s_crc_handle.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
    s_crc_handle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    s_crc_handle.Init.InitValue = 0xFFFFFFFFU;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_crc_handle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  s_crc_handle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;

  if (HAL_CRC_Init(&s_crc_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

uint32_t crc_calculate(const uint32_t *data, size_t length) {
  if (data == NULL) {
    return 0U;
  }

  return HAL_CRC_Calculate(&s_crc_handle, (uint32_t *)data, length);
}

void crc_reset(void) {
  __HAL_RCC_CRC_FORCE_RESET();
  __HAL_RCC_CRC_RELEASE_RESET();
}
