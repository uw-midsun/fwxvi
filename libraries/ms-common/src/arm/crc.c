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
#include "stm32l4xx_hal_crc.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "crc.h"

static CRC_HandleTypeDef s_crc_handle;

StatusCode crc_init(CRCLength crc_length){
    __HAL_RCC_CRC_CLK_ENABLE();

    s_crc_handle.Instance = CRC;
    if (crc_length == 8) {
        s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_8B;
    } else if (crc_length == 16) {
        s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_16B;
    } else if (crc_length == 32) {
        s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_32B;
    } 
    s_crc_handle.Init.DefaultInitValueUse = 0;//fill all these in
    s_crc_handle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    s_crc_handle.Init.InitValue = 0;
    s_crc_handle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE; // no inversion??
    s_crc_handle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;

    if (HAL_CRC_Init(&s_crc_handle) != HAL_OK) {
        return STATUS_CODE_INTERNAL_ERROR; 
    }
    return STATUS_CODE_OK;
}

uint32_t crc_calculate_32(const uint32_t *data, size_t length){
    return HAL_CRC_Calculate(&s_crc_handle, (uint32_t *)data, length);
}
uint32_t crc_calculate_16(const uint16_t *data, size_t length){
    return HAL_CRC_Calculate(&s_crc_handle, (uint16_t *)data, length);
} 
uint32_t crc_calculate_8(const uint8_t *data, size_t length){
    return HAL_CRC_Calculate(&s_crc_handle, (uint8_t *)data, length);
}

void crc_reset(void){
    __HAL_RCC_CRC_FORCE_RESET();
    __HAL_RCC_CRC_RELEASE_RESET();
}