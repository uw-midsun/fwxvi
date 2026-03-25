/************************************************************************************************
 * @file    stm32l4xx_hal_tsc.c
 *
 * @brief   This file provides firmware functions to manage the following
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

/** @defgroup TSC TSC
  * @brief HAL TSC module driver
  * @{
  */

#ifdef HAL_TSC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t TSC_extract_groups(uint32_t iomask);

/* Exported functions --------------------------------------------------------*/

/** @defgroup TSC_Exported_Functions TSC Exported Functions
  * @{
  */

/** @defgroup TSC_Exported_Functions_Group1 Initialization and de-initialization functions
  *  @brief    Initialization and Configuration functions
  *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize and configure the TSC.
      (+) De-initialize the TSC.
@endverbatim
  * @{
  */

/**
  * @brief  Initialize the TSC peripheral according to the specified parameters
  *         in the TSC_InitTypeDef structure and initialize the associated handle.
  * @param  htsc TSC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_Init(TSC_HandleTypeDef *htsc)
{
  /* Check TSC handle allocation */
  if (htsc == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));
  assert_param(IS_TSC_CTPH(htsc->Init.CTPulseHighLength));
  assert_param(IS_TSC_CTPL(htsc->Init.CTPulseLowLength));
  assert_param(IS_TSC_SS(htsc->Init.SpreadSpectrum));
  assert_param(IS_TSC_SSD(htsc->Init.SpreadSpectrumDeviation));
  assert_param(IS_TSC_SS_PRESC(htsc->Init.SpreadSpectrumPrescaler));
  assert_param(IS_TSC_PG_PRESC(htsc->Init.PulseGeneratorPrescaler));
  assert_param(IS_TSC_PG_PRESC_VS_CTPL(htsc->Init.PulseGeneratorPrescaler, htsc->Init.CTPulseLowLength));
  assert_param(IS_TSC_MCV(htsc->Init.MaxCountValue));
  assert_param(IS_TSC_IODEF(htsc->Init.IODefaultMode));
  assert_param(IS_TSC_SYNC_POL(htsc->Init.SynchroPinPolarity));
  assert_param(IS_TSC_ACQ_MODE(htsc->Init.AcquisitionMode));
  assert_param(IS_TSC_MCE_IT(htsc->Init.MaxCountInterrupt));
  assert_param(IS_TSC_GROUP(htsc->Init.ChannelIOs));
  assert_param(IS_TSC_GROUP(htsc->Init.ShieldIOs));
  assert_param(IS_TSC_GROUP(htsc->Init.SamplingIOs));

  if (htsc->State == HAL_TSC_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    htsc->Lock = HAL_UNLOCKED;

#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
    /* Init the TSC Callback settings */
    htsc->ConvCpltCallback  = HAL_TSC_ConvCpltCallback; /* Legacy weak ConvCpltCallback     */
    htsc->ErrorCallback     = HAL_TSC_ErrorCallback;    /* Legacy weak ErrorCallback        */

    if (htsc->MspInitCallback == NULL)
    {
      htsc->MspInitCallback = HAL_TSC_MspInit; /* Legacy weak MspInit  */
    }

    /* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
    htsc->MspInitCallback(htsc);
#else
    /* Init the low level hardware : GPIO, CLOCK, CORTEX */
    HAL_TSC_MspInit(htsc);
#endif /* USE_HAL_TSC_REGISTER_CALLBACKS */
  }

  /* Initialize the TSC state */
  htsc->State = HAL_TSC_STATE_BUSY;

  /*--------------------------------------------------------------------------*/
  /* Set TSC parameters */

  /* Enable TSC */
  htsc->Instance->CR = TSC_CR_TSCE;

  /* Set all functions */
  htsc->Instance->CR |= (htsc->Init.CTPulseHighLength |
                         htsc->Init.CTPulseLowLength |
                         (htsc->Init.SpreadSpectrumDeviation << TSC_CR_SSD_Pos) |
                         htsc->Init.SpreadSpectrumPrescaler |
                         htsc->Init.PulseGeneratorPrescaler |
                         htsc->Init.MaxCountValue |
                         htsc->Init.SynchroPinPolarity |
                         htsc->Init.AcquisitionMode);

  /* Spread spectrum */
  if (htsc->Init.SpreadSpectrum == ENABLE)
  {
    htsc->Instance->CR |= TSC_CR_SSE;
  }

  /* Disable Schmitt trigger hysteresis on all used TSC IOs */
  htsc->Instance->IOHCR = (~(htsc->Init.ChannelIOs | htsc->Init.ShieldIOs | htsc->Init.SamplingIOs));

  /* Set channel and shield IOs */
  htsc->Instance->IOCCR = (htsc->Init.ChannelIOs | htsc->Init.ShieldIOs);

  /* Set sampling IOs */
  htsc->Instance->IOSCR = htsc->Init.SamplingIOs;

  /* Set the groups to be acquired */
  htsc->Instance->IOGCSR = TSC_extract_groups(htsc->Init.ChannelIOs);

  /* Disable interrupts */
  htsc->Instance->IER &= (~(TSC_IT_EOA | TSC_IT_MCE));

  /* Clear flags */
  htsc->Instance->ICR = (TSC_FLAG_EOA | TSC_FLAG_MCE);

  /*--------------------------------------------------------------------------*/

  /* Initialize the TSC state */
  htsc->State = HAL_TSC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Deinitialize the TSC peripheral registers to their default reset values.
  * @param  htsc TSC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_DeInit(TSC_HandleTypeDef *htsc)
{
  /* Check TSC handle allocation */
  if (htsc == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  /* Change TSC state */
  htsc->State = HAL_TSC_STATE_BUSY;

#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
  if (htsc->MspDeInitCallback == NULL)
  {
    htsc->MspDeInitCallback = HAL_TSC_MspDeInit; /* Legacy weak MspDeInit  */
  }

  /* DeInit the low level hardware: GPIO, CLOCK, NVIC */
  htsc->MspDeInitCallback(htsc);
#else
  /* DeInit the low level hardware */
  HAL_TSC_MspDeInit(htsc);
#endif /* USE_HAL_TSC_REGISTER_CALLBACKS */

  /* Change TSC state */
  htsc->State = HAL_TSC_STATE_RESET;

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the TSC MSP.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval None
  */
__weak void HAL_TSC_MspInit(TSC_HandleTypeDef *htsc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htsc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TSC_MspInit could be implemented in the user file.
   */
}

/**
  * @brief  DeInitialize the TSC MSP.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval None
  */
__weak void HAL_TSC_MspDeInit(TSC_HandleTypeDef *htsc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htsc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TSC_MspDeInit could be implemented in the user file.
   */
}

#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User TSC Callback
  *         To be used instead of the weak predefined callback
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *                the configuration information for the specified TSC.
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_TSC_CONV_COMPLETE_CB_ID Conversion completed callback ID
  *          @arg @ref HAL_TSC_ERROR_CB_ID Error callback ID
  *          @arg @ref HAL_TSC_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_TSC_MSPDEINIT_CB_ID MspDeInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_RegisterCallback(TSC_HandleTypeDef *htsc, HAL_TSC_CallbackIDTypeDef CallbackID,
                                           pTSC_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    htsc->ErrorCode |= HAL_TSC_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }
  /* Process locked */
  __HAL_LOCK(htsc);

  if (HAL_TSC_STATE_READY == htsc->State)
  {
    switch (CallbackID)
    {
      case HAL_TSC_CONV_COMPLETE_CB_ID :
        htsc->ConvCpltCallback = pCallback;
        break;

      case HAL_TSC_ERROR_CB_ID :
        htsc->ErrorCallback = pCallback;
        break;

      case HAL_TSC_MSPINIT_CB_ID :
        htsc->MspInitCallback = pCallback;
        break;

      case HAL_TSC_MSPDEINIT_CB_ID :
        htsc->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        htsc->ErrorCode |= HAL_TSC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_TSC_STATE_RESET == htsc->State)
  {
    switch (CallbackID)
    {
      case HAL_TSC_MSPINIT_CB_ID :
        htsc->MspInitCallback = pCallback;
        break;

      case HAL_TSC_MSPDEINIT_CB_ID :
        htsc->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        htsc->ErrorCode |= HAL_TSC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    htsc->ErrorCode |= HAL_TSC_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(htsc);
  return status;
}

/**
  * @brief  Unregister an TSC Callback
  *         TSC callback is redirected to the weak predefined callback
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *                the configuration information for the specified TSC.
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_TSC_CONV_COMPLETE_CB_ID Conversion completed callback ID
  *          @arg @ref HAL_TSC_ERROR_CB_ID Error callback ID
  *          @arg @ref HAL_TSC_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_TSC_MSPDEINIT_CB_ID MspDeInit callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_UnRegisterCallback(TSC_HandleTypeDef *htsc, HAL_TSC_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(htsc);

  if (HAL_TSC_STATE_READY == htsc->State)
  {
    switch (CallbackID)
    {
      case HAL_TSC_CONV_COMPLETE_CB_ID :
        htsc->ConvCpltCallback = HAL_TSC_ConvCpltCallback;       /* Legacy weak ConvCpltCallback      */
        break;

      case HAL_TSC_ERROR_CB_ID :
        htsc->ErrorCallback = HAL_TSC_ErrorCallback;               /* Legacy weak ErrorCallback        */
        break;

      case HAL_TSC_MSPINIT_CB_ID :
        htsc->MspInitCallback = HAL_TSC_MspInit;                   /* Legacy weak MspInit              */
        break;

      case HAL_TSC_MSPDEINIT_CB_ID :
        htsc->MspDeInitCallback = HAL_TSC_MspDeInit;               /* Legacy weak MspDeInit            */
        break;

      default :
        /* Update the error code */
        htsc->ErrorCode |= HAL_TSC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_TSC_STATE_RESET == htsc->State)
  {
    switch (CallbackID)
    {
      case HAL_TSC_MSPINIT_CB_ID :
        htsc->MspInitCallback = HAL_TSC_MspInit;                   /* Legacy weak MspInit              */
        break;

      case HAL_TSC_MSPDEINIT_CB_ID :
        htsc->MspDeInitCallback = HAL_TSC_MspDeInit;               /* Legacy weak MspDeInit            */
        break;

      default :
        /* Update the error code */
        htsc->ErrorCode |= HAL_TSC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    htsc->ErrorCode |= HAL_TSC_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(htsc);
  return status;
}

#endif /* USE_HAL_TSC_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup TSC_Exported_Functions_Group2 Input and Output operation functions
  *  @brief    Input and Output operation functions
  *
@verbatim
 ===============================================================================
             ##### IO Operation functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Start acquisition in polling mode.
      (+) Start acquisition in interrupt mode.
      (+) Stop conversion in polling mode.
      (+) Stop conversion in interrupt mode.
      (+) Poll for acquisition completed.
      (+) Get group acquisition status.
      (+) Get group acquisition value.
@endverbatim
  * @{
  */

/**
  * @brief  Start the acquisition.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_Start(TSC_HandleTypeDef *htsc)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  /* Process locked */
  __HAL_LOCK(htsc);

  /* Change TSC state */
  htsc->State = HAL_TSC_STATE_BUSY;

  /* Clear interrupts */
  __HAL_TSC_DISABLE_IT(htsc, (TSC_IT_EOA | TSC_IT_MCE));

  /* Clear flags */
  __HAL_TSC_CLEAR_FLAG(htsc, (TSC_FLAG_EOA | TSC_FLAG_MCE));

  /* Set touch sensing IOs not acquired to the specified IODefaultMode */
  if (htsc->Init.IODefaultMode == TSC_IODEF_OUT_PP_LOW)
  {
    __HAL_TSC_SET_IODEF_OUTPPLOW(htsc);
  }
  else
  {
    __HAL_TSC_SET_IODEF_INFLOAT(htsc);
  }

  /* Launch the acquisition */
  __HAL_TSC_START_ACQ(htsc);

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the acquisition in interrupt mode.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_TSC_Start_IT(TSC_HandleTypeDef *htsc)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));
  assert_param(IS_TSC_MCE_IT(htsc->Init.MaxCountInterrupt));

  /* Process locked */
  __HAL_LOCK(htsc);

  /* Change TSC state */
  htsc->State = HAL_TSC_STATE_BUSY;

  /* Enable end of acquisition interrupt */
  __HAL_TSC_ENABLE_IT(htsc, TSC_IT_EOA);

  /* Enable max count error interrupt (optional) */
  if (htsc->Init.MaxCountInterrupt == ENABLE)
  {
    __HAL_TSC_ENABLE_IT(htsc, TSC_IT_MCE);
  }
  else
  {
    __HAL_TSC_DISABLE_IT(htsc, TSC_IT_MCE);
  }

  /* Clear flags */
  __HAL_TSC_CLEAR_FLAG(htsc, (TSC_FLAG_EOA | TSC_FLAG_MCE));

  /* Set touch sensing IOs not acquired to the specified IODefaultMode */
  if (htsc->Init.IODefaultMode == TSC_IODEF_OUT_PP_LOW)
  {
    __HAL_TSC_SET_IODEF_OUTPPLOW(htsc);
  }
  else
  {
    __HAL_TSC_SET_IODEF_INFLOAT(htsc);
  }

  /* Launch the acquisition */
  __HAL_TSC_START_ACQ(htsc);

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the acquisition previously launched in polling mode.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_Stop(TSC_HandleTypeDef *htsc)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  /* Process locked */
  __HAL_LOCK(htsc);

  /* Stop the acquisition */
  __HAL_TSC_STOP_ACQ(htsc);

  /* Set touch sensing IOs in low power mode (output push-pull) */
  __HAL_TSC_SET_IODEF_OUTPPLOW(htsc);

  /* Clear flags */
  __HAL_TSC_CLEAR_FLAG(htsc, (TSC_FLAG_EOA | TSC_FLAG_MCE));

  /* Change TSC state */
  htsc->State = HAL_TSC_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the acquisition previously launched in interrupt mode.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_Stop_IT(TSC_HandleTypeDef *htsc)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  /* Process locked */
  __HAL_LOCK(htsc);

  /* Stop the acquisition */
  __HAL_TSC_STOP_ACQ(htsc);

  /* Set touch sensing IOs in low power mode (output push-pull) */
  __HAL_TSC_SET_IODEF_OUTPPLOW(htsc);

  /* Disable interrupts */
  __HAL_TSC_DISABLE_IT(htsc, (TSC_IT_EOA | TSC_IT_MCE));

  /* Clear flags */
  __HAL_TSC_CLEAR_FLAG(htsc, (TSC_FLAG_EOA | TSC_FLAG_MCE));

  /* Change TSC state */
  htsc->State = HAL_TSC_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start acquisition and wait until completion.
  * @note   There is no need of a timeout parameter as the max count error is already
  *         managed by the TSC peripheral.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval HAL state
  */
HAL_StatusTypeDef HAL_TSC_PollForAcquisition(TSC_HandleTypeDef *htsc)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  /* Process locked */
  __HAL_LOCK(htsc);

  /* Check end of acquisition */
  while (HAL_TSC_GetState(htsc) == HAL_TSC_STATE_BUSY)
  {
    /* The timeout (max count error) is managed by the TSC peripheral itself. */
  }

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  return HAL_OK;
}

/**
  * @brief  Get the acquisition status for a group.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @param  gx_index Index of the group
  * @retval Group status
  */
TSC_GroupStatusTypeDef HAL_TSC_GroupGetStatus(const TSC_HandleTypeDef *htsc, uint32_t gx_index)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));
  assert_param(IS_TSC_GROUP_INDEX(gx_index));

  /* Return the group status */
  return (__HAL_TSC_GET_GROUP_STATUS(htsc, gx_index));
}

/**
  * @brief  Get the acquisition measure for a group.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @param  gx_index Index of the group
  * @retval Acquisition measure
  */
uint32_t HAL_TSC_GroupGetValue(const TSC_HandleTypeDef *htsc, uint32_t gx_index)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));
  assert_param(IS_TSC_GROUP_INDEX(gx_index));

  /* Return the group acquisition counter */
  return htsc->Instance->IOGXCR[gx_index];
}

/**
  * @}
  */

/** @defgroup TSC_Exported_Functions_Group3 Peripheral Control functions
  *  @brief    Peripheral Control functions
  *
@verbatim
 ===============================================================================
             ##### Peripheral Control functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Configure TSC IOs
      (+) Discharge TSC IOs
@endverbatim
  * @{
  */

/**
  * @brief  Configure TSC IOs.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @param  config Pointer to the configuration structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_IOConfig(TSC_HandleTypeDef *htsc, const TSC_IOConfigTypeDef *config)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));
  assert_param(IS_TSC_GROUP(config->ChannelIOs));
  assert_param(IS_TSC_GROUP(config->ShieldIOs));
  assert_param(IS_TSC_GROUP(config->SamplingIOs));

  /* Process locked */
  __HAL_LOCK(htsc);

  /* Stop acquisition */
  __HAL_TSC_STOP_ACQ(htsc);

  /* Disable Schmitt trigger hysteresis on all used TSC IOs */
  htsc->Instance->IOHCR = (~(config->ChannelIOs | config->ShieldIOs | config->SamplingIOs));

  /* Set channel and shield IOs */
  htsc->Instance->IOCCR = (config->ChannelIOs | config->ShieldIOs);

  /* Set sampling IOs */
  htsc->Instance->IOSCR = config->SamplingIOs;

  /* Set groups to be acquired */
  htsc->Instance->IOGCSR = TSC_extract_groups(config->ChannelIOs);

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Discharge TSC IOs.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @param  choice This parameter can be set to ENABLE or DISABLE.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_TSC_IODischarge(TSC_HandleTypeDef *htsc, FunctionalState choice)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  /* Process locked */
  __HAL_LOCK(htsc);

  if (choice == ENABLE)
  {
    __HAL_TSC_SET_IODEF_OUTPPLOW(htsc);
  }
  else
  {
    __HAL_TSC_SET_IODEF_INFLOAT(htsc);
  }

  /* Process unlocked */
  __HAL_UNLOCK(htsc);

  /* Return the group acquisition counter */
  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup TSC_Exported_Functions_Group4 Peripheral State and Errors functions
  *  @brief   Peripheral State and Errors functions
  *
@verbatim
 ===============================================================================
            ##### State and Errors functions #####
 ===============================================================================
    [..]
    This subsection provides functions allowing to
      (+) Get TSC state.

@endverbatim
  * @{
  */

/**
  * @brief  Return the TSC handle state.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval HAL state
  */
HAL_TSC_StateTypeDef HAL_TSC_GetState(TSC_HandleTypeDef *htsc)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  if (htsc->State == HAL_TSC_STATE_BUSY)
  {
    /* Check end of acquisition flag */
    if (__HAL_TSC_GET_FLAG(htsc, TSC_FLAG_EOA) != RESET)
    {
      /* Check max count error flag */
      if (__HAL_TSC_GET_FLAG(htsc, TSC_FLAG_MCE) != RESET)
      {
        /* Change TSC state */
        htsc->State = HAL_TSC_STATE_ERROR;
      }
      else
      {
        /* Change TSC state */
        htsc->State = HAL_TSC_STATE_READY;
      }
    }
  }

  /* Return TSC state */
  return htsc->State;
}

/**
  * @}
  */

/** @defgroup TSC_IRQ_Handler_and_Callbacks IRQ Handler and Callbacks
  * @{
  */

/**
  * @brief  Handle TSC interrupt request.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval None
  */
void HAL_TSC_IRQHandler(TSC_HandleTypeDef *htsc)
{
  /* Check the parameters */
  assert_param(IS_TSC_ALL_INSTANCE(htsc->Instance));

  /* Check if the end of acquisition occurred */
  if (__HAL_TSC_GET_FLAG(htsc, TSC_FLAG_EOA) != RESET)
  {
    /* Clear EOA flag */
    __HAL_TSC_CLEAR_FLAG(htsc, TSC_FLAG_EOA);
  }

  /* Check if max count error occurred */
  if (__HAL_TSC_GET_FLAG(htsc, TSC_FLAG_MCE) != RESET)
  {
    /* Clear MCE flag */
    __HAL_TSC_CLEAR_FLAG(htsc, TSC_FLAG_MCE);
    /* Change TSC state */
    htsc->State = HAL_TSC_STATE_ERROR;
#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
    htsc->ErrorCallback(htsc);
#else
    /* Conversion completed callback */
    HAL_TSC_ErrorCallback(htsc);
#endif /* USE_HAL_TSC_REGISTER_CALLBACKS */
  }
  else
  {
    /* Change TSC state */
    htsc->State = HAL_TSC_STATE_READY;
#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
    htsc->ConvCpltCallback(htsc);
#else
    /* Conversion completed callback */
    HAL_TSC_ConvCpltCallback(htsc);
#endif /* USE_HAL_TSC_REGISTER_CALLBACKS */
  }
}

/**
  * @brief  Acquisition completed callback in non-blocking mode.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval None
  */
__weak void HAL_TSC_ConvCpltCallback(TSC_HandleTypeDef *htsc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htsc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TSC_ConvCpltCallback could be implemented in the user file.
   */
}

/**
  * @brief  Error callback in non-blocking mode.
  * @param  htsc Pointer to a TSC_HandleTypeDef structure that contains
  *         the configuration information for the specified TSC.
  * @retval None
  */
__weak void HAL_TSC_ErrorCallback(TSC_HandleTypeDef *htsc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htsc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TSC_ErrorCallback could be implemented in the user file.
   */
}

/**
  * @}
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @defgroup TSC_Private_Functions TSC Private Functions
  * @{
  */

/**
  * @brief  Utility function used to set the acquired groups mask.
  * @param  iomask Channels IOs mask
  * @retval Acquired groups mask
  */
static uint32_t TSC_extract_groups(uint32_t iomask)
{
  uint32_t groups = 0UL;
  uint32_t idx;

  for (idx = 0UL; idx < (uint32_t)TSC_NB_OF_GROUPS; idx++)
  {
    if ((iomask & (0x0FUL << (idx * 4UL))) != 0UL)
    {
      groups |= (1UL << idx);
    }
  }

  return groups;
}

/**
  * @}
  */

#endif /* HAL_TSC_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

