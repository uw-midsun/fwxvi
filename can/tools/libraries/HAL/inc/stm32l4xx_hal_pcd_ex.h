#pragma once

/************************************************************************************************
 * @file    stm32l4xx_hal_pcd_ex.h
 *
 * @brief   Header file of PCD HAL Extension module.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_hal_def.h"

/**
 * @defgroup stm32l4xx_hal_pcd_ex
 * @brief    stm32l4xx_hal_pcd_ex Firmware
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_PCD_EX_H
#define STM32L4xx_HAL_PCD_EX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/

#if defined (USB) || defined (USB_OTG_FS)
/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup PCDEx
  * @{
  */
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup PCDEx_Exported_Functions PCDEx Exported Functions
  * @{
  */
/** @addtogroup PCDEx_Exported_Functions_Group1 Peripheral Control functions
  * @{
  */
#if defined (USB_OTG_FS)
HAL_StatusTypeDef HAL_PCDEx_SetTxFiFo(PCD_HandleTypeDef *hpcd, uint8_t fifo, uint16_t size);
HAL_StatusTypeDef HAL_PCDEx_SetRxFiFo(PCD_HandleTypeDef *hpcd, uint16_t size);
#endif /* defined (USB_OTG_FS) */

#if defined (USB)
HAL_StatusTypeDef  HAL_PCDEx_PMAConfig(PCD_HandleTypeDef *hpcd, uint16_t ep_addr,
                                       uint16_t ep_kind, uint32_t pmaadress);
#endif /* defined (USB) */

HAL_StatusTypeDef HAL_PCDEx_ActivateLPM(PCD_HandleTypeDef *hpcd);
HAL_StatusTypeDef HAL_PCDEx_DeActivateLPM(PCD_HandleTypeDef *hpcd);

HAL_StatusTypeDef HAL_PCDEx_ActivateBCD(PCD_HandleTypeDef *hpcd);
HAL_StatusTypeDef HAL_PCDEx_DeActivateBCD(PCD_HandleTypeDef *hpcd);
void HAL_PCDEx_BCD_VBUSDetect(PCD_HandleTypeDef *hpcd);

void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg);
void HAL_PCDEx_BCD_Callback(PCD_HandleTypeDef *hpcd, PCD_BCD_MsgTypeDef msg);

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
#endif /* defined (USB) || defined (USB_OTG_FS) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STM32L4xx_HAL_PCD_EX_H */

/** @} */
