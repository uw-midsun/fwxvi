/************************************************************************************************
 * @file    stm32l4xx_hal_spi_ex.c
 *
 * @brief   Extended SPI HAL module driver.
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

/** @defgroup SPIEx SPIEx
  * @brief SPI Extended HAL module driver
  * @{
  */
#ifdef HAL_SPI_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/** @defgroup SPIEx_Private_Constants SPIEx Private Constants
  * @{
  */
#define SPI_FIFO_SIZE       4UL
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup SPIEx_Exported_Functions SPIEx Exported Functions
  * @{
  */

/** @defgroup SPIEx_Exported_Functions_Group1 IO operation functions
  *  @brief   Data transfers functions
  *
@verbatim
  ==============================================================================
                      ##### IO operation functions #####
 ===============================================================================
 [..]
    This subsection provides a set of extended functions to manage the SPI
    data transfers.

    (#) Rx data flush function:
        (++) HAL_SPIEx_FlushRxFifo()

@endverbatim
  * @{
  */

/**
  * @brief  Flush the RX fifo.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for the specified SPI module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SPIEx_FlushRxFifo(const SPI_HandleTypeDef *hspi)
{
  __IO uint32_t tmpreg;
  uint8_t  count = 0U;
  while ((hspi->Instance->SR & SPI_FLAG_FRLVL) !=  SPI_FRLVL_EMPTY)
  {
    count++;
    tmpreg = hspi->Instance->DR;
    UNUSED(tmpreg); /* To avoid GCC warning */
    if (count == SPI_FIFO_SIZE)
    {
      return HAL_TIMEOUT;
    }
  }
  return HAL_OK;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_SPI_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */
