#pragma once

/************************************************************************************************
 * @file    stm32l4xx_hal_ltdc_ex.h
 *
 * @brief   Header file of LTDC HAL Extension module.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_dsi.h"

/**
 * @defgroup stm32l4xx_hal_ltdc_ex
 * @brief    stm32l4xx_hal_ltdc_ex Firmware
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_LTDC_EX_H
#define STM32L4xx_HAL_LTDC_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#if defined (LTDC) && defined (DSI)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup LTDCEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup LTDCEx_Exported_Functions
  * @{
  */

/** @addtogroup LTDCEx_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef HAL_LTDCEx_StructInitFromVideoConfig(LTDC_HandleTypeDef *hltdc, DSI_VidCfgTypeDef *VidCfg);
HAL_StatusTypeDef HAL_LTDCEx_StructInitFromAdaptedCommandConfig(LTDC_HandleTypeDef *hltdc, DSI_CmdCfgTypeDef *CmdCfg);
/**
  * @}
  */

/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

#endif /* LTDC && DSI */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_LTDC_EX_H */

/** @} */
