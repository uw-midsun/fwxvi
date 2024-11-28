/************************************************************************************************
 * crc.c
 *
 * CRC Library Source Code
 *
 * Created: 2024-11-20
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "stm32l4xx_hal_crc.h"

/* Intra-component Headers */
#include "crc.h"


#define CRC32_POLYNOMIAL 0x04C11DB7

static CRC_HandleTypeDef s_crc_handle;

static uint32_t s_crc_table[256];

StatusCode crc_init(CRC_HandleTypeDef *hcrc) {
     if (hcrc == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }
    if (DEFAULT_POLYNOMIAL_ENABLE == 0){
    hcrc->Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;  
    hcrc->Init.GeneratingPolynomial = 0x04C11DB7;  
    }
    else hcrc->Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    
    hcrc->Init.CRCLength = CRC_POLYLENGTH_32B;                   
    hcrc->Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;   
    hcrc->Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc->Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc->InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;           

   
    if (HAL_CRC_Init(&s_crc_handle) != HAL_OK) {
        return STATUS_CODE_INTERNAL_ERROR;
    }
    return STATUS_CODE_OK;
}

uint32_t crc_calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength) {
    uint32_t crc = 0xFFFFFFFF;
if (hcrc==NULL||pBuffer==NULL||BufferLength==0) return STATUS_CODE_INVALID_ARGS;
if (HAL_CRC_GetState(hcrc)!=HAL_CRC_STATE_READY) {
        hcrc->HAL_CRC_StateTypeDef  = STATUS_CODE_INTERNAL_ERROR;
        return 0;
    }

    uint32_t crc_result = HAL_CRC_Calculate(hcrc, pBuffer, BufferLength);
    *hcrc->HAL_CRC_StateTypeDef= STATUS_CODE_OK;
    return crc_result;
}