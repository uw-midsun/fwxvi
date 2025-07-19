/************************************************************************************************
 * @file    fota_encryption.c
 *
 * @brief   Fota Encryption Source File
 *
 * @date    2025-04-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_crc.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "fota_encryption.h"

#define FOTA_ENCRYPTION_WORD_SIZE 4U

static CRC_HandleTypeDef s_crc_handle;

FotaError fota_encryption_init() {
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
    return FOTA_ERROR_INTERNAL_ERROR;
  }

  return FOTA_ERROR_SUCCESS;
}

uint32_t fota_calculate_crc32(uint8_t *data_start, uint32_t word_size) {
  if (data_start == NULL) {
    return 0U;
  }

  return HAL_CRC_Calculate(&s_crc_handle, (uint32_t *)data_start, word_size);
}

FotaError fota_verify_packet_encryption(FotaPacket *packet) {
  if (packet == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint32_t received_crc32 = packet->crc32;

  uint8_t *crc32_data_start = (uint8_t *)packet->payload;
  uint32_t crc32_data_size = packet->payload_length;

  /* Handle padding */
  if (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE != 0U) {
    crc32_data_size += FOTA_ENCRYPTION_WORD_SIZE - (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE);
  }

  crc32_data_size /= FOTA_ENCRYPTION_WORD_SIZE; /* Convert bytes to words */

  uint32_t computed_crc32 = fota_calculate_crc32(crc32_data_start, crc32_data_size);
  if (computed_crc32 != received_crc32) {
    return FOTA_ERROR_CRC32_MISMATCH;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_verify_datagram_encryption(FotaDatagram *datagram) {
  if (datagram == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint32_t received_crc32 = datagram->header.datagram_crc32;

  uint8_t *crc32_data_start = (uint8_t *)datagram->data;
  uint32_t crc32_data_size = datagram->header.total_length;

  /* Handle padding */
  if (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE != 0U) {
    crc32_data_size += FOTA_ENCRYPTION_WORD_SIZE - (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE);
  }

  crc32_data_size /= FOTA_ENCRYPTION_WORD_SIZE; /* Convert bytes to words */

  uint32_t computed_crc32 = fota_calculate_crc32(crc32_data_start, crc32_data_size);

  if (computed_crc32 != received_crc32) {
    return FOTA_ERROR_CRC32_MISMATCH;
  }

  return FOTA_ERROR_SUCCESS;
}
