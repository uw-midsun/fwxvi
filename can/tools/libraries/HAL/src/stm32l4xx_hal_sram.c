/************************************************************************************************
 * @file    stm32l4xx_hal_sram.c
 *
 * @brief   SRAM HAL module driver.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_hal.h"

/* Includes ------------------------------------------------------------------*/

#if defined(FMC_BANK1)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

#ifdef HAL_SRAM_MODULE_ENABLED

/** @defgroup SRAM SRAM
  * @brief SRAM driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @addtogroup SRAM_Private_Functions SRAM Private Functions
  * @{
  */
static void SRAM_DMACplt(DMA_HandleTypeDef *hdma);
static void SRAM_DMACpltProt(DMA_HandleTypeDef *hdma);
static void SRAM_DMAError(DMA_HandleTypeDef *hdma);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup SRAM_Exported_Functions SRAM Exported Functions
  * @{
  */

/** @defgroup SRAM_Exported_Functions_Group1 Initialization and de-initialization functions
  * @brief    Initialization and Configuration functions.
  *
  @verbatim
  ==============================================================================
           ##### SRAM Initialization and de_initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to initialize/de-initialize
          the SRAM memory

@endverbatim
  * @{
  */

/**
  * @brief  Performs the SRAM device initialization sequence
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  Timing Pointer to SRAM control timing structure
  * @param  ExtTiming Pointer to SRAM extended mode timing structure
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Init(SRAM_HandleTypeDef *hsram, FMC_NORSRAM_TimingTypeDef *Timing,
                                FMC_NORSRAM_TimingTypeDef *ExtTiming)
{
  /* Check the SRAM handle parameter */
  if (hsram == NULL)
  {
    return HAL_ERROR;
  }

  if (hsram->State == HAL_SRAM_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hsram->Lock = HAL_UNLOCKED;

#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
    if (hsram->MspInitCallback == NULL)
    {
      hsram->MspInitCallback = HAL_SRAM_MspInit;
    }
    hsram->DmaXferCpltCallback = HAL_SRAM_DMA_XferCpltCallback;
    hsram->DmaXferErrorCallback = HAL_SRAM_DMA_XferErrorCallback;

    /* Init the low level hardware */
    hsram->MspInitCallback(hsram);
#else
    /* Initialize the low level hardware (MSP) */
    HAL_SRAM_MspInit(hsram);
#endif /* USE_HAL_SRAM_REGISTER_CALLBACKS */
  }

  /* Initialize SRAM control Interface */
  (void)FMC_NORSRAM_Init(hsram->Instance, &(hsram->Init));

  /* Initialize SRAM timing Interface */
  (void)FMC_NORSRAM_Timing_Init(hsram->Instance, Timing, hsram->Init.NSBank);

  /* Initialize SRAM extended mode timing Interface */
  (void)FMC_NORSRAM_Extended_Timing_Init(hsram->Extended, ExtTiming, hsram->Init.NSBank,
                                         hsram->Init.ExtendedMode);

  /* Enable the NORSRAM device */
  __FMC_NORSRAM_ENABLE(hsram->Instance, hsram->Init.NSBank);

  /* Initialize the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Performs the SRAM device De-initialization sequence.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_DeInit(SRAM_HandleTypeDef *hsram)
{
#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
  if (hsram->MspDeInitCallback == NULL)
  {
    hsram->MspDeInitCallback = HAL_SRAM_MspDeInit;
  }

  /* DeInit the low level hardware */
  hsram->MspDeInitCallback(hsram);
#else
  /* De-Initialize the low level hardware (MSP) */
  HAL_SRAM_MspDeInit(hsram);
#endif /* USE_HAL_SRAM_REGISTER_CALLBACKS */

  /* Configure the SRAM registers with their reset values */
  (void)FMC_NORSRAM_DeInit(hsram->Instance, hsram->Extended, hsram->Init.NSBank);

  /* Reset the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hsram);

  return HAL_OK;
}

/**
  * @brief  SRAM MSP Init.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsram);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_SRAM_MspInit could be implemented in the user file
   */
}

/**
  * @brief  SRAM MSP DeInit.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef *hsram)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsram);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_SRAM_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief  DMA transfer complete callback.
  * @param  hdma pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdma);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_SRAM_DMA_XferCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  DMA transfer complete error callback.
  * @param  hdma pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_DMA_XferErrorCallback(DMA_HandleTypeDef *hdma)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdma);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_SRAM_DMA_XferErrorCallback could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup SRAM_Exported_Functions_Group2 Input Output and memory control functions
  * @brief    Input Output and memory control functions
  *
  @verbatim
  ==============================================================================
                  ##### SRAM Input and Output functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to use and control the SRAM memory

@endverbatim
  * @{
  */

/**
  * @brief  Reads 8-bit buffer from SRAM memory.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_8b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint8_t *pDstBuffer,
                                   uint32_t BufferSize)
{
  uint32_t size;
  __IO uint8_t *psramaddress = (uint8_t *)pAddress;
  uint8_t *pdestbuff = pDstBuffer;
  HAL_SRAM_StateTypeDef state = hsram->State;

  /* Check the SRAM controller state */
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Read data from memory */
    for (size = BufferSize; size != 0U; size--)
    {
      *pdestbuff = *psramaddress;
      pdestbuff++;
      psramaddress++;
    }

    /* Update the SRAM controller state */
    hsram->State = state;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Writes 8-bit buffer to SRAM memory.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_8b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint8_t *pSrcBuffer,
                                    uint32_t BufferSize)
{
  uint32_t size;
  __IO uint8_t *psramaddress = (uint8_t *)pAddress;
  uint8_t *psrcbuff = pSrcBuffer;

  /* Check the SRAM controller state */
  if (hsram->State == HAL_SRAM_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Write data to memory */
    for (size = BufferSize; size != 0U; size--)
    {
      *psramaddress = *psrcbuff;
      psrcbuff++;
      psramaddress++;
    }

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_READY;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Reads 16-bit buffer from SRAM memory.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_16b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint16_t *pDstBuffer,
                                    uint32_t BufferSize)
{
  uint32_t size;
  __IO uint32_t *psramaddress = pAddress;
  uint16_t *pdestbuff = pDstBuffer;
  uint8_t limit;
  HAL_SRAM_StateTypeDef state = hsram->State;

  /* Check the SRAM controller state */
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Check if the size is a 32-bits multiple */
    limit = (((BufferSize % 2U) != 0U) ? 1U : 0U);

    /* Read data from memory */
    for (size = BufferSize; size != limit; size -= 2U)
    {
      *pdestbuff = (uint16_t)((*psramaddress) & 0x0000FFFFU);
      pdestbuff++;
      *pdestbuff = (uint16_t)(((*psramaddress) & 0xFFFF0000U) >> 16U);
      pdestbuff++;
      psramaddress++;
    }

    /* Read last 16-bits if size is not 32-bits multiple */
    if (limit != 0U)
    {
      *pdestbuff = (uint16_t)((*psramaddress) & 0x0000FFFFU);
    }

    /* Update the SRAM controller state */
    hsram->State = state;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Writes 16-bit buffer to SRAM memory.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_16b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint16_t *pSrcBuffer,
                                     uint32_t BufferSize)
{
  uint32_t size;
  __IO uint32_t *psramaddress = pAddress;
  uint16_t *psrcbuff = pSrcBuffer;
  uint8_t limit;

  /* Check the SRAM controller state */
  if (hsram->State == HAL_SRAM_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Check if the size is a 32-bits multiple */
    limit = (((BufferSize % 2U) != 0U) ? 1U : 0U);

    /* Write data to memory */
    for (size = BufferSize; size != limit; size -= 2U)
    {
      *psramaddress = (uint32_t)(*psrcbuff);
      psrcbuff++;
      *psramaddress |= ((uint32_t)(*psrcbuff) << 16U);
      psrcbuff++;
      psramaddress++;
    }

    /* Write last 16-bits if size is not 32-bits multiple */
    if (limit != 0U)
    {
      *psramaddress = ((uint32_t)(*psrcbuff) & 0x0000FFFFU) | ((*psramaddress) & 0xFFFF0000U);
    }

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_READY;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Reads 32-bit buffer from SRAM memory.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_32b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pDstBuffer,
                                    uint32_t BufferSize)
{
  uint32_t size;
  __IO uint32_t *psramaddress = pAddress;
  uint32_t *pdestbuff = pDstBuffer;
  HAL_SRAM_StateTypeDef state = hsram->State;

  /* Check the SRAM controller state */
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Read data from memory */
    for (size = BufferSize; size != 0U; size--)
    {
      *pdestbuff = *psramaddress;
      pdestbuff++;
      psramaddress++;
    }

    /* Update the SRAM controller state */
    hsram->State = state;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Writes 32-bit buffer to SRAM memory.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_32b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pSrcBuffer,
                                     uint32_t BufferSize)
{
  uint32_t size;
  __IO uint32_t *psramaddress = pAddress;
  uint32_t *psrcbuff = pSrcBuffer;

  /* Check the SRAM controller state */
  if (hsram->State == HAL_SRAM_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Write data to memory */
    for (size = BufferSize; size != 0U; size--)
    {
      *psramaddress = *psrcbuff;
      psrcbuff++;
      psramaddress++;
    }

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_READY;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Reads a Words data from the SRAM memory using DMA transfer.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_DMA(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pDstBuffer,
                                    uint32_t BufferSize)
{
  HAL_StatusTypeDef status;
  HAL_SRAM_StateTypeDef state = hsram->State;

  /* Check the SRAM controller state */
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Configure DMA user callbacks */
    if (state == HAL_SRAM_STATE_READY)
    {
      hsram->hdma->XferCpltCallback = SRAM_DMACplt;
    }
    else
    {
      hsram->hdma->XferCpltCallback = SRAM_DMACpltProt;
    }
    hsram->hdma->XferErrorCallback = SRAM_DMAError;

    /* Enable the DMA Stream */
    status = HAL_DMA_Start_IT(hsram->hdma, (uint32_t)pAddress, (uint32_t)pDstBuffer, (uint32_t)BufferSize);

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    status = HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Writes a Words data buffer to SRAM memory using DMA transfer.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_DMA(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pSrcBuffer,
                                     uint32_t BufferSize)
{
  HAL_StatusTypeDef status;

  /* Check the SRAM controller state */
  if (hsram->State == HAL_SRAM_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Configure DMA user callbacks */
    hsram->hdma->XferCpltCallback = SRAM_DMACplt;
    hsram->hdma->XferErrorCallback = SRAM_DMAError;

    /* Enable the DMA Stream */
    status = HAL_DMA_Start_IT(hsram->hdma, (uint32_t)pSrcBuffer, (uint32_t)pAddress, (uint32_t)BufferSize);

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    status = HAL_ERROR;
  }

  return status;
}

#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User SRAM Callback
  *         To be used to override the weak predefined callback
  * @param hsram : SRAM handle
  * @param CallbackId : ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_SRAM_MSP_INIT_CB_ID       SRAM MspInit callback ID
  *          @arg @ref HAL_SRAM_MSP_DEINIT_CB_ID     SRAM MspDeInit callback ID
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_SRAM_RegisterCallback(SRAM_HandleTypeDef *hsram, HAL_SRAM_CallbackIDTypeDef CallbackId,
                                            pSRAM_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SRAM_StateTypeDef state;

  if (pCallback == NULL)
  {
    return HAL_ERROR;
  }

  state = hsram->State;
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_RESET) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    switch (CallbackId)
    {
      case HAL_SRAM_MSP_INIT_CB_ID :
        hsram->MspInitCallback = pCallback;
        break;
      case HAL_SRAM_MSP_DEINIT_CB_ID :
        hsram->MspDeInitCallback = pCallback;
        break;
      default :
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* update return status */
    status =  HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Unregister a User SRAM Callback
  *         SRAM Callback is redirected to the weak predefined callback
  * @param hsram : SRAM handle
  * @param CallbackId : ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_SRAM_MSP_INIT_CB_ID       SRAM MspInit callback ID
  *          @arg @ref HAL_SRAM_MSP_DEINIT_CB_ID     SRAM MspDeInit callback ID
  *          @arg @ref HAL_SRAM_DMA_XFER_CPLT_CB_ID  SRAM DMA Xfer Complete callback ID
  *          @arg @ref HAL_SRAM_DMA_XFER_ERR_CB_ID   SRAM DMA Xfer Error callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_SRAM_UnRegisterCallback(SRAM_HandleTypeDef *hsram, HAL_SRAM_CallbackIDTypeDef CallbackId)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SRAM_StateTypeDef state;

  state = hsram->State;
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    switch (CallbackId)
    {
      case HAL_SRAM_MSP_INIT_CB_ID :
        hsram->MspInitCallback = HAL_SRAM_MspInit;
        break;
      case HAL_SRAM_MSP_DEINIT_CB_ID :
        hsram->MspDeInitCallback = HAL_SRAM_MspDeInit;
        break;
      case HAL_SRAM_DMA_XFER_CPLT_CB_ID :
        hsram->DmaXferCpltCallback = HAL_SRAM_DMA_XferCpltCallback;
        break;
      case HAL_SRAM_DMA_XFER_ERR_CB_ID :
        hsram->DmaXferErrorCallback = HAL_SRAM_DMA_XferErrorCallback;
        break;
      default :
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (state == HAL_SRAM_STATE_RESET)
  {
    switch (CallbackId)
    {
      case HAL_SRAM_MSP_INIT_CB_ID :
        hsram->MspInitCallback = HAL_SRAM_MspInit;
        break;
      case HAL_SRAM_MSP_DEINIT_CB_ID :
        hsram->MspDeInitCallback = HAL_SRAM_MspDeInit;
        break;
      default :
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* update return status */
    status =  HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Register a User SRAM Callback for DMA transfers
  *         To be used to override the weak predefined callback
  * @param hsram : SRAM handle
  * @param CallbackId : ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_SRAM_DMA_XFER_CPLT_CB_ID  SRAM DMA Xfer Complete callback ID
  *          @arg @ref HAL_SRAM_DMA_XFER_ERR_CB_ID   SRAM DMA Xfer Error callback ID
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_SRAM_RegisterDmaCallback(SRAM_HandleTypeDef *hsram, HAL_SRAM_CallbackIDTypeDef CallbackId,
                                               pSRAM_DmaCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SRAM_StateTypeDef state;

  if (pCallback == NULL)
  {
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hsram);

  state = hsram->State;
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    switch (CallbackId)
    {
      case HAL_SRAM_DMA_XFER_CPLT_CB_ID :
        hsram->DmaXferCpltCallback = pCallback;
        break;
      case HAL_SRAM_DMA_XFER_ERR_CB_ID :
        hsram->DmaXferErrorCallback = pCallback;
        break;
      default :
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsram);
  return status;
}
#endif /* USE_HAL_SRAM_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup SRAM_Exported_Functions_Group3 Control functions
  *  @brief   Control functions
  *
@verbatim
  ==============================================================================
                        ##### SRAM Control functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to control dynamically
    the SRAM interface.

@endverbatim
  * @{
  */

/**
  * @brief  Enables dynamically SRAM write operation.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_WriteOperation_Enable(SRAM_HandleTypeDef *hsram)
{
  /* Check the SRAM controller state */
  if (hsram->State == HAL_SRAM_STATE_PROTECTED)
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Enable write operation */
    (void)FMC_NORSRAM_WriteOperation_Enable(hsram->Instance, hsram->Init.NSBank);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_READY;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Disables dynamically SRAM write operation.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_WriteOperation_Disable(SRAM_HandleTypeDef *hsram)
{
  /* Check the SRAM controller state */
  if (hsram->State == HAL_SRAM_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Disable write operation */
    (void)FMC_NORSRAM_WriteOperation_Disable(hsram->Instance, hsram->Init.NSBank);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_PROTECTED;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup SRAM_Exported_Functions_Group4 Peripheral State functions
  *  @brief   Peripheral State functions
  *
@verbatim
  ==============================================================================
                      ##### SRAM State functions #####
  ==============================================================================
  [..]
    This subsection permits to get in run-time the status of the SRAM controller
    and the data flow.

@endverbatim
  * @{
  */

/**
  * @brief  Returns the SRAM controller state
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL state
  */
HAL_SRAM_StateTypeDef HAL_SRAM_GetState(const SRAM_HandleTypeDef *hsram)
{
  return hsram->State;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup SRAM_Private_Functions SRAM Private Functions
  * @{
  */

/**
  * @brief  DMA SRAM process complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void SRAM_DMACplt(DMA_HandleTypeDef *hdma)
{
  /* Derogation MISRAC2012-Rule-11.5 */
  SRAM_HandleTypeDef *hsram = (SRAM_HandleTypeDef *)(hdma->Parent);

  /* Disable the DMA channel */
  __HAL_DMA_DISABLE(hdma);

  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY;

#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
  hsram->DmaXferCpltCallback(hdma);
#else
  HAL_SRAM_DMA_XferCpltCallback(hdma);
#endif /* USE_HAL_SRAM_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA SRAM process complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void SRAM_DMACpltProt(DMA_HandleTypeDef *hdma)
{
  /* Derogation MISRAC2012-Rule-11.5 */
  SRAM_HandleTypeDef *hsram = (SRAM_HandleTypeDef *)(hdma->Parent);

  /* Disable the DMA channel */
  __HAL_DMA_DISABLE(hdma);

  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_PROTECTED;

#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
  hsram->DmaXferCpltCallback(hdma);
#else
  HAL_SRAM_DMA_XferCpltCallback(hdma);
#endif /* USE_HAL_SRAM_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA SRAM error callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void SRAM_DMAError(DMA_HandleTypeDef *hdma)
{
  /* Derogation MISRAC2012-Rule-11.5 */
  SRAM_HandleTypeDef *hsram = (SRAM_HandleTypeDef *)(hdma->Parent);

  /* Disable the DMA channel */
  __HAL_DMA_DISABLE(hdma);

  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_ERROR;

#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
  hsram->DmaXferErrorCallback(hdma);
#else
  HAL_SRAM_DMA_XferErrorCallback(hdma);
#endif /* USE_HAL_SRAM_REGISTER_CALLBACKS */
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_SRAM_MODULE_ENABLED */

/**
  * @}
  */

#endif /* FMC_BANK1 */
