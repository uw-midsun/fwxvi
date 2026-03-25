/************************************************************************************************
 * @file    stm32l4xx_ll_crs.h
 *
 * @brief   CRS LL module driver.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_crs.h"
#if defined(USE_FULL_LL_DRIVER)

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(CRS)

/** @defgroup CRS_LL CRS
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup CRS_LL_Exported_Functions
  * @{
  */

/** @addtogroup CRS_LL_EF_Init
  * @{
  */

/**
  * @brief  De-Initializes CRS peripheral registers to their default reset values.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: CRS registers are de-initialized
  *          - ERROR: not applicable
  */
ErrorStatus LL_CRS_DeInit(void)
{
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_CRS);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_CRS);

  return  SUCCESS;
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

#endif /* defined(CRS) */

/**
  * @}
  */
  
#endif /* USE_FULL_LL_DRIVER */
