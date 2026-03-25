#pragma once

/************************************************************************************************
 * @file    stm32l4xx_hal_sai_ex.h
 *
 * @brief   Header file of SAI HAL extended module.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_hal_def.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_SAI_EX_H
#define STM32L4xx_HAL_SAI_EX_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx) || \
    defined(STM32L4P5xx) || defined(STM32L4Q5xx)

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup SAIEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup SAIEx_Exported_Types SAIEx Exported Types
  * @{
  */

/**
  * @brief  PDM microphone delay structure definition
  */
typedef struct
{
  uint32_t MicPair;     /*!< Specifies which pair of microphones is selected.
                             This parameter must be a number between Min_Data = 1 and Max_Data = 3. */

  uint32_t LeftDelay;   /*!< Specifies the delay in PDM clock unit to apply on left microphone.
                             This parameter must be a number between Min_Data = 0 and Max_Data = 7. */

  uint32_t RightDelay;  /*!< Specifies the delay in PDM clock unit to apply on right microphone.
                             This parameter must be a number between Min_Data = 0 and Max_Data = 7. */
} SAIEx_PdmMicDelayParamTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup SAIEx_Exported_Functions SAIEx Extended Exported Functions
  * @{
  */

/** @addtogroup SAIEx_Exported_Functions_Group1 Peripheral Control functions
  * @{
  */
HAL_StatusTypeDef HAL_SAIEx_ConfigPdmMicDelay(const SAI_HandleTypeDef *hsai,
                                              const SAIEx_PdmMicDelayParamTypeDef *pdmMicDelay);
/**
  * @}
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @addtogroup SAIEx_Private_Macros SAIEx Extended Private Macros
  * @{
  */
#define IS_SAI_PDM_MIC_DELAY(VALUE)   ((VALUE) <= 7U)
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx || */
/* STM32L4P5xx || STM32L4Q5xx */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_SAI_EX_H */

