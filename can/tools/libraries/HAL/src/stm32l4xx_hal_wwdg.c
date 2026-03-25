/************************************************************************************************
 * @file    stm32l4xx_hal_wwdg.c
 *
 * @brief   WWDG HAL module driver.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_hal.h"

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

#ifdef HAL_WWDG_MODULE_ENABLED
/** @defgroup WWDG WWDG
  * @brief WWDG HAL module driver.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup WWDG_Exported_Functions WWDG Exported Functions
  * @{
  */

/** @defgroup WWDG_Exported_Functions_Group1 Initialization and Configuration functions
  *  @brief    Initialization and Configuration functions.
  *
@verbatim
  ==============================================================================
          ##### Initialization and Configuration functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to:
      (+) Initialize and start the WWDG according to the specified parameters
          in the WWDG_InitTypeDef of associated handle.
      (+) Initialize the WWDG MSP.

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the WWDG according to the specified.
  *         parameters in the WWDG_InitTypeDef of  associated handle.
  * @param  hwwdg  pointer to a WWDG_HandleTypeDef structure that contains
  *                the configuration information for the specified WWDG module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_WWDG_Init(WWDG_HandleTypeDef *hwwdg)
{
  /* Check the WWDG handle allocation */
  if (hwwdg == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_WWDG_ALL_INSTANCE(hwwdg->Instance));
  assert_param(IS_WWDG_PRESCALER(hwwdg->Init.Prescaler));
  assert_param(IS_WWDG_WINDOW(hwwdg->Init.Window));
  assert_param(IS_WWDG_COUNTER(hwwdg->Init.Counter));
  assert_param(IS_WWDG_EWI_MODE(hwwdg->Init.EWIMode));

#if (USE_HAL_WWDG_REGISTER_CALLBACKS == 1)
  /* Reset Callback pointers */
  if (hwwdg->EwiCallback == NULL)
  {
    hwwdg->EwiCallback = HAL_WWDG_EarlyWakeupCallback;
  }

  if (hwwdg->MspInitCallback == NULL)
  {
    hwwdg->MspInitCallback = HAL_WWDG_MspInit;
  }

  /* Init the low level hardware */
  hwwdg->MspInitCallback(hwwdg);
#else
  /* Init the low level hardware */
  HAL_WWDG_MspInit(hwwdg);
#endif /* USE_HAL_WWDG_REGISTER_CALLBACKS */

  /* Set WWDG Counter */
  WRITE_REG(hwwdg->Instance->CR, (WWDG_CR_WDGA | hwwdg->Init.Counter));

  /* Set WWDG Prescaler and Window */
  WRITE_REG(hwwdg->Instance->CFR, (hwwdg->Init.EWIMode | hwwdg->Init.Prescaler | hwwdg->Init.Window));

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the WWDG MSP.
  * @param  hwwdg  pointer to a WWDG_HandleTypeDef structure that contains
  *                the configuration information for the specified WWDG module.
  * @note   When rewriting this function in user file, mechanism may be added
  *         to avoid multiple initialize when HAL_WWDG_Init function is called
  *         again to change parameters.
  * @retval None
  */
__weak void HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hwwdg);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_WWDG_MspInit could be implemented in the user file
   */
}

#if (USE_HAL_WWDG_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User WWDG Callback
  *         To be used instead of the weak (surcharged) predefined callback
  * @param  hwwdg WWDG handle
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_WWDG_EWI_CB_ID Early WakeUp Interrupt Callback ID
  *           @arg @ref HAL_WWDG_MSPINIT_CB_ID MspInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_WWDG_RegisterCallback(WWDG_HandleTypeDef *hwwdg, HAL_WWDG_CallbackIDTypeDef CallbackID,
                                            pWWDG_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    status = HAL_ERROR;
  }
  else
  {
    switch (CallbackID)
    {
      case HAL_WWDG_EWI_CB_ID:
        hwwdg->EwiCallback = pCallback;
        break;

      case HAL_WWDG_MSPINIT_CB_ID:
        hwwdg->MspInitCallback = pCallback;
        break;

      default:
        status = HAL_ERROR;
        break;
    }
  }

  return status;
}

/**
  * @brief  Unregister a WWDG Callback
  *         WWDG Callback is redirected to the weak (surcharged) predefined callback
  * @param  hwwdg WWDG handle
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_WWDG_EWI_CB_ID Early WakeUp Interrupt Callback ID
  *           @arg @ref HAL_WWDG_MSPINIT_CB_ID MspInit callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_WWDG_UnRegisterCallback(WWDG_HandleTypeDef *hwwdg, HAL_WWDG_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  switch (CallbackID)
  {
    case HAL_WWDG_EWI_CB_ID:
      hwwdg->EwiCallback = HAL_WWDG_EarlyWakeupCallback;
      break;

    case HAL_WWDG_MSPINIT_CB_ID:
      hwwdg->MspInitCallback = HAL_WWDG_MspInit;
      break;

    default:
      status = HAL_ERROR;
      break;
  }

  return status;
}
#endif /* USE_HAL_WWDG_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup WWDG_Exported_Functions_Group2 IO operation functions
  *  @brief    IO operation functions
  *
@verbatim
  ==============================================================================
                      ##### IO operation functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to:
    (+) Refresh the WWDG.
    (+) Handle WWDG interrupt request and associated function callback.

@endverbatim
  * @{
  */

/**
  * @brief  Refresh the WWDG.
  * @param  hwwdg  pointer to a WWDG_HandleTypeDef structure that contains
  *                the configuration information for the specified WWDG module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_WWDG_Refresh(WWDG_HandleTypeDef *hwwdg)
{
  /* Write to WWDG CR the WWDG Counter value to refresh with */
  WRITE_REG(hwwdg->Instance->CR, (hwwdg->Init.Counter));

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Handle WWDG interrupt request.
  * @note   The Early Wakeup Interrupt (EWI) can be used if specific safety operations
  *         or data logging must be performed before the actual reset is generated.
  *         The EWI interrupt is enabled by calling HAL_WWDG_Init function with
  *         EWIMode set to WWDG_EWI_ENABLE.
  *         When the downcounter reaches the value 0x40, and EWI interrupt is
  *         generated and the corresponding Interrupt Service Routine (ISR) can
  *         be used to trigger specific actions (such as communications or data
  *         logging), before resetting the device.
  * @param  hwwdg  pointer to a WWDG_HandleTypeDef structure that contains
  *                the configuration information for the specified WWDG module.
  * @retval None
  */
void HAL_WWDG_IRQHandler(WWDG_HandleTypeDef *hwwdg)
{
  /* Check if Early Wakeup Interrupt is enable */
  if (__HAL_WWDG_GET_IT_SOURCE(hwwdg, WWDG_IT_EWI) != RESET)
  {
    /* Check if WWDG Early Wakeup Interrupt occurred */
    if (__HAL_WWDG_GET_FLAG(hwwdg, WWDG_FLAG_EWIF) != RESET)
    {
      /* Clear the WWDG Early Wakeup flag */
      __HAL_WWDG_CLEAR_FLAG(hwwdg, WWDG_FLAG_EWIF);

#if (USE_HAL_WWDG_REGISTER_CALLBACKS == 1)
      /* Early Wakeup registered callback */
      hwwdg->EwiCallback(hwwdg);
#else
      /* Early Wakeup callback */
      HAL_WWDG_EarlyWakeupCallback(hwwdg);
#endif /* USE_HAL_WWDG_REGISTER_CALLBACKS */
    }
  }
}

/**
  * @brief  WWDG Early Wakeup callback.
  * @param  hwwdg  pointer to a WWDG_HandleTypeDef structure that contains
  *                the configuration information for the specified WWDG module.
  * @retval None
  */
__weak void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hwwdg);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_WWDG_EarlyWakeupCallback could be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_WWDG_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */
