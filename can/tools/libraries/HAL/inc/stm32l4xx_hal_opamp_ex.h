#pragma once

/************************************************************************************************
 * @file    stm32l4xx_hal_opamp_ex.h
 *
 * @brief   Header file of OPAMP HAL Extended module.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_hal_def.h"

/**
 * @defgroup stm32l4xx_hal_opamp_ex
 * @brief    stm32l4xx_hal_opamp_ex Firmware
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_OPAMP_EX_H
#define STM32L4xx_HAL_OPAMP_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup OPAMPEx
  * @{
  */
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup OPAMPEx_Exported_Functions OPAMPEx Exported Functions
  * @{
  */

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)

/* I/O operation functions  *****************************************************/
/** @addtogroup OPAMPEx_Exported_Functions_Group1 Extended Input and Output operation functions
  * @{
  */

HAL_StatusTypeDef HAL_OPAMPEx_SelfCalibrateAll(OPAMP_HandleTypeDef *hopamp1, OPAMP_HandleTypeDef *hopamp2);

/**
  * @}
  */
#endif

/* Peripheral Control functions  ************************************************/
/** @addtogroup OPAMPEx_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef HAL_OPAMPEx_Unlock(OPAMP_HandleTypeDef *hopamp);
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

#endif /* STM32L4xx_HAL_OPAMP_EX_H */

/** @} */
