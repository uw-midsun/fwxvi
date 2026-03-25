/************************************************************************************************
 * @file   mcu.c
 *
 * @brief  Source code for MCU intialization
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "gpio.h"
#include "mcu.h"
#include "retarget.h"

/* Intra-component Headers */
#include "stm32l4xx_hal.h"

static StatusCode SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0U };
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0U };

#if USE_INTERNAL_OSCILLATOR == 1U
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
#else
  /* Enable oscillators: HSE (16MHz) and LSE (32.768kHz) */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;

  /* Configure PLL with HSE as source */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  /* PLL calculation for 80MHz from 16MHz HSE:
   * 16MHz / PLLM(2) = 8MHz
   * 8MHz * PLLN(20) = 160MHz
   * 160MHz / PLLR(2) = 80MHz SYSCLK
   */
  RCC_OscInitStruct.PLL.PLLM = 2;  /* Divide */
  RCC_OscInitStruct.PLL.PLLN = 20; /* Multiply */
  RCC_OscInitStruct.PLL.PLLR = 2;  /* Divide */

  /* Initialize PLLP and PLLQ even though they’re not used */
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 2;
#endif

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    /* Initialization Error */
    return STATUS_CODE_INTERNAL_ERROR;
  }

#if USE_INTERNAL_OSCILLATOR == 0U
  uint32_t start_time = HAL_GetTick();

  while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) != SET && __HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != SET) {
    if (HAL_GetTick() - start_time > HSE_LSE_TIMEOUT_MS) {
      return STATUS_CODE_TIMEOUT;
    }
  }
#endif

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    /* Initialization Error */
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode mcu_init(void) {
  if (HAL_Init() != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  gpio_init();
  retarget_init();

  return SystemClock_Config();
}
