/************************************************************************************************
 * @file    stm32l4xx_hal_dac.c
 *
 * @brief   DAC HAL module driver.
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

#ifdef HAL_DAC_MODULE_ENABLED
#if defined(DAC1)

  /** @defgroup DAC DAC
  * @brief DAC driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @addtogroup DAC_Private_Constants DAC Private Constants
  * @{
  */
#define TIMEOUT_DAC_CALIBCONFIG        1U         /* 1   ms        */
#define HFSEL_ENABLE_THRESHOLD_80MHZ   80000000U  /* 80 MHz        */

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/

/** @defgroup DAC_Exported_Functions DAC Exported Functions
  * @{
  */

/** @defgroup DAC_Exported_Functions_Group1 Initialization and de-initialization functions
 *  @brief    Initialization and Configuration functions
 *
@verbatim
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize and configure the DAC.
      (+) De-initialize the DAC.

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the DAC peripheral according to the specified parameters
  *         in the DAC_InitStruct and initialize the associated handle.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Init(DAC_HandleTypeDef *hdac)
{
  /* Check DAC handle */
  if (hdac == NULL)
  {
     return HAL_ERROR;
  }
  /* Check the parameters */
  assert_param(IS_DAC_ALL_INSTANCE(hdac->Instance));

  if (hdac->State == HAL_DAC_STATE_RESET)
  {
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    /* Init the DAC Callback settings */
    hdac->ConvCpltCallbackCh1           = HAL_DAC_ConvCpltCallbackCh1;
    hdac->ConvHalfCpltCallbackCh1       = HAL_DAC_ConvHalfCpltCallbackCh1;
    hdac->ErrorCallbackCh1              = HAL_DAC_ErrorCallbackCh1;
    hdac->DMAUnderrunCallbackCh1        = HAL_DAC_DMAUnderrunCallbackCh1;

#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
    hdac->ConvCpltCallbackCh2           = HAL_DACEx_ConvCpltCallbackCh2;
    hdac->ConvHalfCpltCallbackCh2       = HAL_DACEx_ConvHalfCpltCallbackCh2;
    hdac->ErrorCallbackCh2              = HAL_DACEx_ErrorCallbackCh2;
    hdac->DMAUnderrunCallbackCh2        = HAL_DACEx_DMAUnderrunCallbackCh2;
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4P5xx STM32L4Q5xx                                                             */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */

    if (hdac->MspInitCallback == NULL)
    {
      hdac->MspInitCallback             = HAL_DAC_MspInit;
    }
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

    /* Allocate lock resource and initialize it */
    hdac->Lock = HAL_UNLOCKED;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    /* Init the low level hardware */
    hdac->MspInitCallback(hdac);
#else
    /* Init the low level hardware */
    HAL_DAC_MspInit(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
  }

  /* Initialize the DAC state*/
  hdac->State = HAL_DAC_STATE_BUSY;

  /* Set DAC error code to none */
  hdac->ErrorCode = HAL_DAC_ERROR_NONE;

  /* Initialize the DAC state*/
  hdac->State = HAL_DAC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Deinitialize the DAC peripheral registers to their default reset values.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_DeInit(DAC_HandleTypeDef *hdac)
{
  /* Check DAC handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_ALL_INSTANCE(hdac->Instance));

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
  if (hdac->MspDeInitCallback == NULL)
  {
    hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
  }
  /* DeInit the low level hardware */
  hdac->MspDeInitCallback(hdac);
#else
  /* DeInit the low level hardware */
  HAL_DAC_MspDeInit(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

  /* Set DAC error code to none */
  hdac->ErrorCode = HAL_DAC_ERROR_NONE;

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the DAC MSP.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitialize the DAC MSP.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_MspDeInit could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup DAC_Exported_Functions_Group2 IO operation functions
 *  @brief    IO operation functions
 *
@verbatim
  ==============================================================================
             ##### IO operation functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Start conversion.
      (+) Stop conversion.
      (+) Start conversion and enable DMA transfer.
      (+) Stop conversion and disable DMA transfer.
      (+) Get result of conversion.

@endverbatim
  * @{
  */

/**
  * @brief  Enables DAC and starts conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected (when supported)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Start(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  /* Enable the Peripheral */
  __HAL_DAC_ENABLE(hdac, Channel);

#if defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
  if (Channel == DAC_CHANNEL_1)
  {
    /* Check if software trigger enabled */
    if ((hdac->Instance->CR & (DAC_CR_TEN1 | DAC_CR_TSEL1)) == DAC_TRIGGER_SOFTWARE)
    {
      /* Enable the selected DAC software conversion */
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG1);
    }
  }
  else
  {
    /* Check if software trigger enabled */
    if ((hdac->Instance->CR & (DAC_CR_TEN2 | DAC_CR_TSEL2)) == (DAC_TRIGGER_SOFTWARE << (Channel & 0x10UL)))
    {
      /* Enable the selected DAC software conversion*/
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG2);
    }
  }

#endif /* STM32L4P5xx STM32L4Q5xx STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx                                     */

#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx)
  if(Channel == DAC_CHANNEL_1)
  {
    /* Check if software trigger enabled */
    if ((hdac->Instance->CR & (DAC_CR_TEN1 | DAC_CR_TSEL1)) == DAC_CR_TEN1)
    {
      /* Enable the selected DAC software conversion */
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG1);
    }
  }
  else
  {
    /* Check if software trigger enabled */
    if ((hdac->Instance->CR & (DAC_CR_TEN2 | DAC_CR_TSEL2)) == DAC_CR_TEN2)
    {
      /* Enable the selected DAC software conversion*/
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG2);
    }
  }
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */

#if defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx)
  /* Check if software trigger enabled */
  if((hdac->Instance->CR & (DAC_CR_TEN1 | DAC_CR_TSEL1)) == (DAC_CR_TEN1 | DAC_CR_TSEL1))
  {
    /* Enable the selected DAC software conversion */
    SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG1);
  }
#endif /* STM32L451xx STM32L452xx STM32L462xx */
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Disables DAC and stop conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Stop(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Disable the Peripheral */
  __HAL_DAC_DISABLE(hdac, Channel);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

#if defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx)
/**
  * @brief  Enables DAC and starts conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  * @param  pData The destination peripheral Buffer address.
  * @param  Length The length of data to be transferred from memory to DAC peripheral
  * @param  Alignment Specifies the data alignment for DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Start_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t *pData, uint32_t Length,
                                    uint32_t Alignment)
{
  HAL_StatusTypeDef status;
  uint32_t tmpreg = 0U;

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_ALIGN(Alignment));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  /* Set the DMA transfer complete callback for channel1 */
  hdac->DMA_Handle1->XferCpltCallback = DAC_DMAConvCpltCh1;

  /* Set the DMA half transfer complete callback for channel1 */
  hdac->DMA_Handle1->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh1;

  /* Set the DMA error callback for channel1 */
  hdac->DMA_Handle1->XferErrorCallback = DAC_DMAErrorCh1;

  /* Enable the selected DAC channel1 DMA request */
  SET_BIT(hdac->Instance->CR, DAC_CR_DMAEN1);

  /* Case of use of channel 1 */
  switch (Alignment)
  {
    case DAC_ALIGN_12B_R:
      /* Get DHR12R1 address */
      tmpreg = (uint32_t)&hdac->Instance->DHR12R1;
      break;
    case DAC_ALIGN_12B_L:
      /* Get DHR12L1 address */
      tmpreg = (uint32_t)&hdac->Instance->DHR12L1;
      break;
    case DAC_ALIGN_8B_R:
      /* Get DHR8R1 address */
      tmpreg = (uint32_t)&hdac->Instance->DHR8R1;
      break;
    default:
      break;
  }

  /* Enable the DMA channel */
  /* Enable the DAC DMA underrun interrupt */
  __HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR1);

  /* Enable the DMA channel */
  status = HAL_DMA_Start_IT(hdac->DMA_Handle1, (uint32_t)pData, tmpreg, Length);

  /* Process Unlocked */
  __HAL_UNLOCK(hdac);

  if (status == HAL_OK)
  {
    /* Enable the Peripheral */
    __HAL_DAC_ENABLE(hdac, Channel);
  }
  else
  {
    hdac->ErrorCode |= HAL_DAC_ERROR_DMA;
  }

  /* Return function status */
  return status;
}
#endif /* STM32L451xx STM32L452xx STM32L462xx */

#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)

/**
  * @brief  Enables DAC and starts conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  pData The destination peripheral Buffer address.
  * @param  Length The length of data to be transferred from memory to DAC peripheral
  * @param  Alignment Specifies the data alignment for DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Start_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t *pData, uint32_t Length,
                                    uint32_t Alignment)
{
  HAL_StatusTypeDef status;
  uint32_t tmpreg = 0U;

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_ALIGN(Alignment));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  if (Channel == DAC_CHANNEL_1)
  {
    /* Set the DMA transfer complete callback for channel1 */
    hdac->DMA_Handle1->XferCpltCallback = DAC_DMAConvCpltCh1;

    /* Set the DMA half transfer complete callback for channel1 */
    hdac->DMA_Handle1->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh1;

    /* Set the DMA error callback for channel1 */
    hdac->DMA_Handle1->XferErrorCallback = DAC_DMAErrorCh1;

    /* Enable the selected DAC channel1 DMA request */
    SET_BIT(hdac->Instance->CR, DAC_CR_DMAEN1);

    /* Case of use of channel 1 */
    switch (Alignment)
    {
      case DAC_ALIGN_12B_R:
        /* Get DHR12R1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12R1;
        break;
      case DAC_ALIGN_12B_L:
        /* Get DHR12L1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12L1;
        break;
      case DAC_ALIGN_8B_R:
        /* Get DHR8R1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR8R1;
        break;
      default:
        break;
    }
  }
  else
  {
    /* Set the DMA transfer complete callback for channel2 */
    hdac->DMA_Handle2->XferCpltCallback = DAC_DMAConvCpltCh2;

    /* Set the DMA half transfer complete callback for channel2 */
    hdac->DMA_Handle2->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh2;

    /* Set the DMA error callback for channel2 */
    hdac->DMA_Handle2->XferErrorCallback = DAC_DMAErrorCh2;

    /* Enable the selected DAC channel2 DMA request */
    SET_BIT(hdac->Instance->CR, DAC_CR_DMAEN2);

    /* Case of use of channel 2 */
    switch (Alignment)
    {
      case DAC_ALIGN_12B_R:
        /* Get DHR12R2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12R2;
        break;
      case DAC_ALIGN_12B_L:
        /* Get DHR12L2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12L2;
        break;
      case DAC_ALIGN_8B_R:
        /* Get DHR8R2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR8R2;
        break;
      default:
        break;
    }
  }

  /* Enable the DMA channel */
  if (Channel == DAC_CHANNEL_1)
  {
    /* Enable the DAC DMA underrun interrupt */
    __HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR1);

    /* Enable the DMA channel */
    status = HAL_DMA_Start_IT(hdac->DMA_Handle1, (uint32_t)pData, tmpreg, Length);
  }
  else
  {
    /* Enable the DAC DMA underrun interrupt */
    __HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR2);

    /* Enable the DMA channel */
    status = HAL_DMA_Start_IT(hdac->DMA_Handle2, (uint32_t)pData, tmpreg, Length);
  }

  /* Process Unlocked */
  __HAL_UNLOCK(hdac);

  if (status == HAL_OK)
  {
    /* Enable the Peripheral */
    __HAL_DAC_ENABLE(hdac, Channel);
  }
  else
  {
    hdac->ErrorCode |= HAL_DAC_ERROR_DMA;
  }

  /* Return function status */
  return status;
}
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4P5xx STM32L4Q5xx                                                             */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */

/**
  * @brief  Disables DAC and stop conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Stop_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Disable the selected DAC channel DMA request */
  hdac->Instance->CR &= ~(DAC_CR_DMAEN1 << (Channel & 0x10UL));

  /* Disable the Peripheral */
  __HAL_DAC_DISABLE(hdac, Channel);

  /* Disable the DMA channel */
#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
  /* Channel1 is used */
  if (Channel == DAC_CHANNEL_1)
  {
    /* Disable the DMA channel */
    (void)HAL_DMA_Abort(hdac->DMA_Handle1);

    /* Disable the DAC DMA underrun interrupt */
    __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR1);
  }
  else /* Channel2 is used for */
  {
    /* Disable the DMA channel */
    (void)HAL_DMA_Abort(hdac->DMA_Handle2);

    /* Disable the DAC DMA underrun interrupt */
    __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR2);
  }
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4P5xx STM32L4Q5xx                                                             */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */

#if defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx)
  /* Disable the DMA channel */
  (void)HAL_DMA_Abort(hdac->DMA_Handle1);

  /* Disable the DAC DMA underrun interrupt */
  __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR1);
#endif /* STM32L451xx STM32L452xx STM32L462xx */

  /* Return function status */
  return HAL_OK;
}

/* DAC channel 2 is available on top of DAC channel 1 in */
/* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
/* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */

/**
  * @brief  Handles DAC interrupt request
  *         This function uses the interruption of DMA
  *         underrun.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
void HAL_DAC_IRQHandler(DAC_HandleTypeDef *hdac)
{
  if (__HAL_DAC_GET_IT_SOURCE(hdac, DAC_IT_DMAUDR1))
  {
    /* Check underrun flag of DAC channel 1 */
    if (__HAL_DAC_GET_FLAG(hdac, DAC_FLAG_DMAUDR1))
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;

      /* Set DAC error code to chanel1 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH1);

      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac, DAC_FLAG_DMAUDR1);

      /* Disable the selected DAC channel1 DMA request */
      CLEAR_BIT(hdac->Instance->CR, DAC_CR_DMAEN1);

      /* Error callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
      hdac->DMAUnderrunCallbackCh1(hdac);
#else
      HAL_DAC_DMAUnderrunCallbackCh1(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
    }
  }
#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
  if(__HAL_DAC_GET_IT_SOURCE(hdac, DAC_IT_DMAUDR2))
  {
    /* Check underrun flag of DAC channel 2 */
    if (__HAL_DAC_GET_FLAG(hdac, DAC_FLAG_DMAUDR2))
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;

      /* Set DAC error code to channel2 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH2);

      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac, DAC_FLAG_DMAUDR2);

      /* Disable the selected DAC channel2 DMA request */
      CLEAR_BIT(hdac->Instance->CR, DAC_CR_DMAEN2);

      /* Error callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
      hdac->DMAUnderrunCallbackCh2(hdac);
#else
      HAL_DACEx_DMAUnderrunCallbackCh2(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
    }
  }
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4P5xx STM32L4Q5xx                                                             */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */
}

/**
  * @brief  Set the specified data holding register value for DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  Alignment Specifies the data alignment.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @param  Data Data to be loaded in the selected data holding register.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_SetValue(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t Alignment, uint32_t Data)
{
  __IO uint32_t tmp = 0;

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_ALIGN(Alignment));
  assert_param(IS_DAC_DATA(Data));

  tmp = (uint32_t)hdac->Instance;
  if (Channel == DAC_CHANNEL_1)
  {
    tmp += DAC_DHR12R1_ALIGNMENT(Alignment);
  }
  else
  {
    tmp += DAC_DHR12R2_ALIGNMENT(Alignment);
  }

  /* Set the DAC channel selected data holding register */
  *(__IO uint32_t *) tmp = Data;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Conversion complete callback in non-blocking mode for Channel1
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ConvCpltCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  Conversion half DMA transfer callback in non-blocking mode for Channel1
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ConvHalfCpltCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  Error DAC callback for Channel1.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ErrorCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  DMA underrun DAC callback for channel1.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_DMAUnderrunCallbackCh1 could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup DAC_Exported_Functions_Group3 Peripheral Control functions
 *  @brief    Peripheral Control functions
 *
@verbatim
  ==============================================================================
             ##### Peripheral Control functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Configure channels.
      (+) Set the specified data holding register value for DAC channel.

@endverbatim
  * @{
  */

/**
  * @brief  Returns the last data output value of the selected DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval The selected DAC channel data output value.
  */
uint32_t HAL_DAC_GetValue(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Returns the DAC channel data output register value */
#if defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx)
  /* Prevent unused argument(s) compilation warning if no assert_param check */
  UNUSED(Channel);

  return hdac->Instance->DOR1;
#endif /* STM32L451xx STM32L452xx STM32L462xx */

#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
  if(Channel == DAC_CHANNEL_1)
  {
    return hdac->Instance->DOR1;
  }
  else
  {
    return hdac->Instance->DOR2;
  }
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4P5xx STM32L4Q5xx                                                             */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */
}

/**
  * @brief  Configures the selected DAC channel.
  * @note   By calling this function, the high frequency interface mode (HFSEL bits)
  *         will be set. This parameter scope is the DAC instance. As the function
  *         is called for each channel, the @ref DAC_HighFrequency of @arg sConfig
  *         must be the same at each call.
  *         (or DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC self detect).
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  sConfig DAC configuration structure.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected (Whenever present)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_ConfigChannel(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel)
{
  uint32_t tmpreg1;
  uint32_t tmpreg2;
  uint32_t tickstart = 0U;
#if defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
  uint32_t hclkfreq;
#endif /* STM32L4P5xx STM32L4Q5xx STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx */

  /* Check the DAC parameters */
#if defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
  assert_param(IS_DAC_HIGH_FREQUENCY_MODE(sConfig->DAC_HighFrequency));
#endif /* STM32L4P5xx STM32L4Q5xx STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx */
  assert_param(IS_DAC_TRIGGER(sConfig->DAC_Trigger));
  assert_param(IS_DAC_OUTPUT_BUFFER_STATE(sConfig->DAC_OutputBuffer));
  assert_param(IS_DAC_CHIP_CONNECTION(sConfig->DAC_ConnectOnChipPeripheral));
  assert_param(IS_DAC_TRIMMING(sConfig->DAC_UserTrimming));
  if ((sConfig->DAC_UserTrimming) == DAC_TRIMMING_USER)
  {
    assert_param(IS_DAC_TRIMMINGVALUE(sConfig->DAC_TrimmingValue));
  }
  assert_param(IS_DAC_SAMPLEANDHOLD(sConfig->DAC_SampleAndHold));
  if ((sConfig->DAC_SampleAndHold) == DAC_SAMPLEANDHOLD_ENABLE)
  {
    assert_param(IS_DAC_SAMPLETIME(sConfig->DAC_SampleAndHoldConfig.DAC_SampleTime));
    assert_param(IS_DAC_HOLDTIME(sConfig->DAC_SampleAndHoldConfig.DAC_HoldTime));
    assert_param(IS_DAC_REFRESHTIME(sConfig->DAC_SampleAndHoldConfig.DAC_RefreshTime));
  }
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  if (sConfig->DAC_SampleAndHold == DAC_SAMPLEANDHOLD_ENABLE)
  /* Sample on old configuration */
  {
    /* Get timeout */
    tickstart = HAL_GetTick();

    if (Channel == DAC_CHANNEL_1)
    {

      /* SHSR1 can be written when BWST1 is cleared */
      while (((hdac->Instance->SR) & DAC_SR_BWST1) != 0UL)
      {
        /* Check for the Timeout */
        if ((HAL_GetTick() - tickstart) > TIMEOUT_DAC_CALIBCONFIG)
        {
          /* New check to avoid false timeout detection in case of preemption */
          if(((hdac->Instance->SR) & DAC_SR_BWST1) != 0UL)
          {
            /* Update error code */
            SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_TIMEOUT);

            /* Change the DMA state */
            hdac->State = HAL_DAC_STATE_TIMEOUT;

            return HAL_TIMEOUT;
          }
        }
      }
      HAL_Delay(1);
      hdac->Instance->SHSR1 = sConfig->DAC_SampleAndHoldConfig.DAC_SampleTime;
    }
#if !defined (STM32L451xx) & !defined (STM32L452xx) & !defined (STM32L462xx)
    else /* Channel 2 */
    {
      /* SHSR2 can be written when BWST2 is cleared */

      while (((hdac->Instance->SR) & DAC_SR_BWST2) != 0UL)
      {
        /* Check for the Timeout */
        if ((HAL_GetTick() - tickstart) > TIMEOUT_DAC_CALIBCONFIG)
        {
          /* New check to avoid false timeout detection in case of preemption */
          if(((hdac->Instance->SR) & DAC_SR_BWST2) != 0UL)
          {
            /* Update error code */
            SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_TIMEOUT);

            /* Change the DMA state */
            hdac->State = HAL_DAC_STATE_TIMEOUT;

            return HAL_TIMEOUT;
          }
        }
      }
      HAL_Delay(1U);
      hdac->Instance->SHSR2 = sConfig->DAC_SampleAndHoldConfig.DAC_SampleTime;
    }
#endif /* STM32L451xx STM32L452xx STM32L462xx */

    /* HoldTime */
    MODIFY_REG(hdac->Instance->SHHR, DAC_SHHR_THOLD1 << (Channel & 0x10UL), (sConfig->DAC_SampleAndHoldConfig.DAC_HoldTime) << (Channel & 0x10UL));
    /* RefreshTime */
    MODIFY_REG(hdac->Instance->SHRR, DAC_SHRR_TREFRESH1 << (Channel & 0x10UL), (sConfig->DAC_SampleAndHoldConfig.DAC_RefreshTime) << (Channel & 0x10UL));
  }

  if (sConfig->DAC_UserTrimming == DAC_TRIMMING_USER)
  /* USER TRIMMING */
  {
    /* Get the DAC CCR value */
    tmpreg1 = hdac->Instance->CCR;
    /* Clear trimming value */
    tmpreg1 &= ~(((uint32_t)(DAC_CCR_OTRIM1)) << (Channel & 0x10UL));
    /* Configure for the selected trimming offset */
    tmpreg2 = sConfig->DAC_TrimmingValue;
    /* Calculate CCR register value depending on DAC_Channel */
    tmpreg1 |= tmpreg2 << (Channel & 0x10UL);
    /* Write to DAC CCR */
    hdac->Instance->CCR = tmpreg1;
  }
  /* else factory trimming is used (factory setting are available at reset)*/
  /* SW Nothing has nothing to do */

  /* Get the DAC MCR value */
  tmpreg1 = hdac->Instance->MCR;
  /* Clear DAC_MCR_MODEx bits */
  tmpreg1 &= ~(((uint32_t)(DAC_MCR_MODE1)) << (Channel & 0x10UL));
  /* Configure for the selected DAC channel: mode, buffer output & on chip peripheral connect */
  tmpreg2 = (sConfig->DAC_SampleAndHold | sConfig->DAC_OutputBuffer | sConfig->DAC_ConnectOnChipPeripheral);
  /* Calculate MCR register value depending on DAC_Channel */
  tmpreg1 |= tmpreg2 << (Channel & 0x10UL);
  /* Write to DAC MCR */
  hdac->Instance->MCR = tmpreg1;

  /* DAC in normal operating mode hence clear DAC_CR_CENx bit */
  CLEAR_BIT(hdac->Instance->CR, DAC_CR_CEN1 << (Channel & 0x10UL));

  /* Get the DAC CR value */
  tmpreg1 = hdac->Instance->CR;
  /* Clear TENx, TSELx, WAVEx and MAMPx bits */
  tmpreg1 &= ~(((uint32_t)(DAC_CR_MAMP1 | DAC_CR_WAVE1 | DAC_CR_TSEL1 | DAC_CR_TEN1)) << (Channel & 0x10UL));
  /* Configure for the selected DAC channel: trigger */
  /* Set TSELx and TENx bits according to DAC_Trigger value */
  tmpreg2 = sConfig->DAC_Trigger;
  /* Calculate CR register value depending on DAC_Channel */
  tmpreg1 |= tmpreg2 << (Channel & 0x10UL);
#if defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
  if (DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ == sConfig->DAC_HighFrequency)
  {
    tmpreg1 |= DAC_CR_HFSEL;
  }
  else
  {
    if (DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE == sConfig->DAC_HighFrequency)
    {
      tmpreg1 &= ~(DAC_CR_HFSEL);
    }
    else /* Automatic selection */
    {
      hclkfreq = HAL_RCC_GetHCLKFreq();
      if (hclkfreq > HFSEL_ENABLE_THRESHOLD_80MHZ)
      {
        /* High frequency enable when HCLK frequency higher than 80   */
         tmpreg1 |= DAC_CR_HFSEL;
      }
      else
      {
        /* High frequency disable when HCLK frequency higher than 80  */
        tmpreg1 &= ~(DAC_CR_HFSEL);
      }
    }
  }

#endif /* STM32L4P5xx STM32L4Q5xx STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx */

  /* Write to DAC CR */
  hdac->Instance->CR = tmpreg1;
  /* Disable wave generation */
  hdac->Instance->CR &= ~(DAC_CR_WAVE1 << (Channel & 0x10UL));

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup DAC_Exported_Functions_Group4 Peripheral State and Errors functions
 *  @brief   Peripheral State and Errors functions
 *
@verbatim
  ==============================================================================
            ##### Peripheral State and Errors functions #####
  ==============================================================================
    [..]
    This subsection provides functions allowing to
      (+) Check the DAC state.
      (+) Check the DAC Errors.

@endverbatim
  * @{
  */

/**
  * @brief  return the DAC handle state
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL state
  */
HAL_DAC_StateTypeDef HAL_DAC_GetState(DAC_HandleTypeDef *hdac)
{
  /* Return DAC handle state */
  return hdac->State;
}

/**
  * @brief  Return the DAC error code
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval DAC Error Code
  */
uint32_t HAL_DAC_GetError(DAC_HandleTypeDef *hdac)
{
  return hdac->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup DAC_Exported_Functions
  * @{
  */

/** @addtogroup DAC_Exported_Functions_Group1
  * @{
  */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User DAC Callback
  *         To be used instead of the weak (overridden) predefined callback
  * @param  hdac DAC handle
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_DAC_ERROR_INVALID_CALLBACK   DAC Error Callback ID
  *          @arg @ref HAL_DAC_CH1_COMPLETE_CB_ID       DAC CH1 Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_HALF_COMPLETE_CB_ID  DAC CH1 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_ERROR_ID             DAC CH1 Error Callback ID
  *          @arg @ref HAL_DAC_CH1_UNDERRUN_CB_ID       DAC CH1 UnderRun Callback ID
  *          @arg @ref HAL_DAC_CH2_COMPLETE_CB_ID       DAC CH2 Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_HALF_COMPLETE_CB_ID  DAC CH2 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_ERROR_ID             DAC CH2 Error Callback ID
  *          @arg @ref HAL_DAC_CH2_UNDERRUN_CB_ID       DAC CH2 UnderRun Callback ID
  *          @arg @ref HAL_DAC_MSPINIT_CB_ID            DAC MSP Init Callback ID
  *          @arg @ref HAL_DAC_MSPDEINIT_CB_ID          DAC MSP DeInit Callback ID
  *
  * @param  pCallback pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_DAC_RegisterCallback(DAC_HandleTypeDef *hdac, HAL_DAC_CallbackIDTypeDef CallbackID,
                                           pDAC_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hdac);

  if (hdac->State == HAL_DAC_STATE_READY)
  {
    switch (CallbackID)
    {
    case HAL_DAC_CH1_COMPLETE_CB_ID :
      hdac->ConvCpltCallbackCh1 = pCallback;
      break;
    case HAL_DAC_CH1_HALF_COMPLETE_CB_ID :
      hdac->ConvHalfCpltCallbackCh1 = pCallback;
      break;
    case HAL_DAC_CH1_ERROR_ID :
      hdac->ErrorCallbackCh1 = pCallback;
      break;
    case HAL_DAC_CH1_UNDERRUN_CB_ID :
      hdac->DMAUnderrunCallbackCh1 = pCallback;
      break;
    case HAL_DAC_CH2_COMPLETE_CB_ID :
      hdac->ConvCpltCallbackCh2 = pCallback;
      break;
    case HAL_DAC_CH2_HALF_COMPLETE_CB_ID :
      hdac->ConvHalfCpltCallbackCh2 = pCallback;
      break;
    case HAL_DAC_CH2_ERROR_ID :
      hdac->ErrorCallbackCh2 = pCallback;
      break;
    case HAL_DAC_CH2_UNDERRUN_CB_ID :
      hdac->DMAUnderrunCallbackCh2 = pCallback;
      break;
    case HAL_DAC_MSPINIT_CB_ID :
      hdac->MspInitCallback = pCallback;
      break;
    case HAL_DAC_MSPDEINIT_CB_ID :
      hdac->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hdac->State == HAL_DAC_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_DAC_MSPINIT_CB_ID :
      hdac->MspInitCallback = pCallback;
      break;
    case HAL_DAC_MSPDEINIT_CB_ID :
      hdac->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hdac);
  return status;
}

/**
  * @brief  Unregister a User DAC Callback
  *         DAC Callback is redirected to the weak (overridden) predefined callback
  * @param  hdac DAC handle
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_DAC_CH1_COMPLETE_CB_ID          DAC CH1 transfer Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_HALF_COMPLETE_CB_ID     DAC CH1 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_ERROR_ID                DAC CH1 Error Callback ID
  *          @arg @ref HAL_DAC_CH1_UNDERRUN_CB_ID          DAC CH1 UnderRun Callback ID
  *          @arg @ref HAL_DAC_CH2_COMPLETE_CB_ID          DAC CH2 Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_HALF_COMPLETE_CB_ID     DAC CH2 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_ERROR_ID                DAC CH2 Error Callback ID
  *          @arg @ref HAL_DAC_CH2_UNDERRUN_CB_ID          DAC CH2 UnderRun Callback ID
  *          @arg @ref HAL_DAC_MSPINIT_CB_ID               DAC MSP Init Callback ID
  *          @arg @ref HAL_DAC_MSPDEINIT_CB_ID             DAC MSP DeInit Callback ID
  *          @arg @ref HAL_DAC_ALL_CB_ID                   DAC All callbacks
  * @retval status
  */
HAL_StatusTypeDef HAL_DAC_UnRegisterCallback(DAC_HandleTypeDef *hdac, HAL_DAC_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hdac);

  if (hdac->State == HAL_DAC_STATE_READY)
  {
    switch (CallbackID)
    {
    case HAL_DAC_CH1_COMPLETE_CB_ID :
      hdac->ConvCpltCallbackCh1 = HAL_DAC_ConvCpltCallbackCh1;
      break;
    case HAL_DAC_CH1_HALF_COMPLETE_CB_ID :
      hdac->ConvHalfCpltCallbackCh1 = HAL_DAC_ConvHalfCpltCallbackCh1;
      break;
    case HAL_DAC_CH1_ERROR_ID :
      hdac->ErrorCallbackCh1 = HAL_DAC_ErrorCallbackCh1;
      break;
    case HAL_DAC_CH1_UNDERRUN_CB_ID :
      hdac->DMAUnderrunCallbackCh1 = HAL_DAC_DMAUnderrunCallbackCh1;
      break;
#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
    case HAL_DAC_CH2_COMPLETE_CB_ID :
      hdac->ConvCpltCallbackCh2 = HAL_DACEx_ConvCpltCallbackCh2;
      break;
    case HAL_DAC_CH2_HALF_COMPLETE_CB_ID :
      hdac->ConvHalfCpltCallbackCh2 = HAL_DACEx_ConvHalfCpltCallbackCh2;
      break;
    case HAL_DAC_CH2_ERROR_ID :
      hdac->ErrorCallbackCh2 = HAL_DACEx_ErrorCallbackCh2;
      break;
    case HAL_DAC_CH2_UNDERRUN_CB_ID :
      hdac->DMAUnderrunCallbackCh2 = HAL_DACEx_DMAUnderrunCallbackCh2;
      break;
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4P5xx STM32L4Q5xx                                                             */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */
    case HAL_DAC_MSPINIT_CB_ID :
      hdac->MspInitCallback = HAL_DAC_MspInit;
      break;
    case HAL_DAC_MSPDEINIT_CB_ID :
      hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
      break;
    case HAL_DAC_ALL_CB_ID :
      hdac->ConvCpltCallbackCh1 = HAL_DAC_ConvCpltCallbackCh1;
      hdac->ConvHalfCpltCallbackCh1 = HAL_DAC_ConvHalfCpltCallbackCh1;
      hdac->ErrorCallbackCh1 = HAL_DAC_ErrorCallbackCh1;
      hdac->DMAUnderrunCallbackCh1 = HAL_DAC_DMAUnderrunCallbackCh1;
#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
      hdac->ConvCpltCallbackCh2 = HAL_DACEx_ConvCpltCallbackCh2;
      hdac->ConvHalfCpltCallbackCh2 = HAL_DACEx_ConvHalfCpltCallbackCh2;
      hdac->ErrorCallbackCh2 = HAL_DACEx_ErrorCallbackCh2;
      hdac->DMAUnderrunCallbackCh2 = HAL_DACEx_DMAUnderrunCallbackCh2;
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4P5xx STM32L4Q5xx                                                             */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */
      hdac->MspInitCallback = HAL_DAC_MspInit;
      hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
      break;
    default :
      /* Update the error code */
      hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hdac->State == HAL_DAC_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_DAC_MSPINIT_CB_ID :
      hdac->MspInitCallback = HAL_DAC_MspInit;
      break;
    case HAL_DAC_MSPDEINIT_CB_ID :
      hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
      break;
    default :
      /* Update the error code */
      hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hdac);
  return status;
}
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup DAC_Private_Functions
  * @{
  */

/**
  * @brief  DMA conversion complete callback.
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAConvCpltCh1(DMA_HandleTypeDef *hdma)
{
  DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
  hdac->ConvCpltCallbackCh1(hdac);
#else
  HAL_DAC_ConvCpltCallbackCh1(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

  hdac->State = HAL_DAC_STATE_READY;
}

/**
  * @brief  DMA half transfer complete callback.
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAHalfConvCpltCh1(DMA_HandleTypeDef *hdma)
{
  DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  /* Conversion complete callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
  hdac->ConvHalfCpltCallbackCh1(hdac);
#else
  HAL_DAC_ConvHalfCpltCallbackCh1(hdac);
#endif  /* USE_HAL_DAC_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA error callback
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAErrorCh1(DMA_HandleTypeDef *hdma)
{
  DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Set DAC error code to DMA error */
  hdac->ErrorCode |= HAL_DAC_ERROR_DMA;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
  hdac->ErrorCallbackCh1(hdac);
#else
  HAL_DAC_ErrorCallbackCh1(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

  hdac->State = HAL_DAC_STATE_READY;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* DAC1 */

#endif /* HAL_DAC_MODULE_ENABLED */

/**
  * @}
  */

