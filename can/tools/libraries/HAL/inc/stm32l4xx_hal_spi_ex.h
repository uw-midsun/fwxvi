#pragma once

/************************************************************************************************
 * @file    stm32l4xx_hal_spi_ex.h
 *
 * @brief   Header file of SPI HAL Extended module.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_hal_def.h"

/**
 * @defgroup stm32l4xx_hal_spi_ex
 * @brief    stm32l4xx_hal_spi_ex Firmware
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_SPI_EX_H
#define STM32L4xx_HAL_SPI_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup SPIEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup SPIEx_Exported_Functions
  * @{
  */

/* Initialization and de-initialization functions  ****************************/
/* IO operation functions *****************************************************/
/** @addtogroup SPIEx_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef HAL_SPIEx_FlushRxFifo(const SPI_HandleTypeDef *hspi);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_SPI_EX_H */


/** @} */
