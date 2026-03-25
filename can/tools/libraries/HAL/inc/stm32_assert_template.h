#pragma once

/************************************************************************************************
 * @file    stm32_assert.h
 *
 * @brief   STM32 assert template file.
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup stm32_assert_template
 * @brief    stm32_assert_template Firmware
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32_ASSERT_H
#define STM32_ASSERT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((char *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(char *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* STM32_ASSERT_H */

/** @} */
