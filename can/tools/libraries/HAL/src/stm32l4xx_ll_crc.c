/************************************************************************************************
 * @file    stm32l4xx_ll_crc.c
 *
 * @brief   CRC LL module driver.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32_assert.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_crc.h"
#if defined(USE_FULL_LL_DRIVER)

/* Includes ------------------------------------------------------------------*/

#ifdef  USE_FULL_ASSERT
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined (CRC)

/** @addtogroup CRC_LL
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup CRC_LL_Exported_Functions
  * @{
  */

/** @addtogroup CRC_LL_EF_Init
  * @{
  */

/**
  * @brief  De-initialize CRC registers (Registers restored to their default values).
  * @param  CRCx CRC Instance
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: CRC registers are de-initialized
  *          - ERROR: CRC registers are not de-initialized
  */
ErrorStatus LL_CRC_DeInit(const CRC_TypeDef *CRCx)
{
  ErrorStatus status = SUCCESS;

  /* Check the parameters */
  assert_param(IS_CRC_ALL_INSTANCE(CRCx));

  if (CRCx == CRC)
  {
    /* Force CRC reset */
    LL_AHB1_GRP1_ForceReset(LL_AHB1_GRP1_PERIPH_CRC);

    /* Release CRC reset */
    LL_AHB1_GRP1_ReleaseReset(LL_AHB1_GRP1_PERIPH_CRC);
  }
  else
  {
    status = ERROR;
  }

  return (status);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* defined (CRC) */

/**
  * @}
  */

#endif /* USE_FULL_LL_DRIVER */
