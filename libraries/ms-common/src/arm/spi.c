/************************************************************************************************
 * @file   spi.c
 *
 * @brief  SPI Library Source Code
 *
 * @date   2024-12-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_spi.h"

/* Intra-component Headers */
#include "interrupts.h"
#include "spi.h"

static inline void s_enable_spi1(void) {
  __HAL_RCC_SPI1_CLK_ENABLE();
}
static inline void s_enable_spi2(void) {
  __HAL_RCC_SPI2_CLK_ENABLE();
}

static inline void s_enable_spi3(void) {
  __HAL_RCC_SPI3_CLK_ENABLE();
}

/** @brief  SPI Port data */
typedef struct {
  SPI_TypeDef *base;     /**< SPI HW Base address */
  void (*rcc_cmd)(void); /**< Function pointer to enable SPI clock using RCC */
  uint8_t irq;           /**< SPI interrupt number */
  bool initialized;      /**< Initialized flag */
} SPIPortData;

static SPIPortData s_port[NUM_SPI_PORTS] = {
  [SPI_PORT_1] = { .rcc_cmd = s_enable_spi1, .base = SPI1, .irq = SPI1_IRQn },
  [SPI_PORT_2] = { .rcc_cmd = s_enable_spi2, .base = SPI2, .irq = SPI2_IRQn },
  [SPI_PORT_3] = { .rcc_cmd = s_enable_spi3, .base = SPI3, .irq = SPI3_IRQn },
};

static SPI_HandleTypeDef s_spi_handles[NUM_SPI_PORTS];

/* Mutex for port access */
static StaticSemaphore_t s_spi_port_mutex[NUM_SPI_PORTS];
static SemaphoreHandle_t s_spi_port_handle[NUM_SPI_PORTS];

/* Semaphore to signal event complete */
static StaticSemaphore_t s_spi_cmplt_sem[NUM_SPI_PORTS];
static SemaphoreHandle_t s_spi_cmplt_handle[NUM_SPI_PORTS];

void SPI1_IRQHandler(void) {
  HAL_SPI_IRQHandler(&s_spi_handles[SPI_PORT_1]);
}

void SPI2_IRQHandler(void) {
  HAL_SPI_IRQHandler(&s_spi_handles[SPI_PORT_2]);
}

void SPI3_IRQHandler(void) {
  HAL_SPI_IRQHandler(&s_spi_handles[SPI_PORT_3]);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (hspi->Instance == SPI1) {
    xSemaphoreGiveFromISR(s_spi_cmplt_handle[SPI_PORT_1], &xHigherPriorityTaskWoken);
  } else if (hspi->Instance == SPI2) {
    xSemaphoreGiveFromISR(s_spi_cmplt_handle[SPI_PORT_2], &xHigherPriorityTaskWoken);
  } else {
    xSemaphoreGiveFromISR(s_spi_cmplt_handle[SPI_PORT_3], &xHigherPriorityTaskWoken);
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (hspi->Instance == SPI1) {
    xSemaphoreGiveFromISR(s_spi_cmplt_handle[SPI_PORT_1], &xHigherPriorityTaskWoken);
  } else if (hspi->Instance == SPI2) {
    xSemaphoreGiveFromISR(s_spi_cmplt_handle[SPI_PORT_2], &xHigherPriorityTaskWoken);
  } else {
    xSemaphoreGiveFromISR(s_spi_cmplt_handle[SPI_PORT_3], &xHigherPriorityTaskWoken);
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (spi >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_port[spi].initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  s_spi_port_handle[spi] = xSemaphoreCreateMutexStatic(&s_spi_port_mutex[spi]);
  s_spi_cmplt_handle[spi] = xSemaphoreCreateBinaryStatic(&s_spi_cmplt_sem[spi]);

  if (s_spi_port_handle[spi] == NULL || s_spi_cmplt_handle[spi] == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (spi == SPI_PORT_3) {
    gpio_init_pin_af(&settings->sdo, GPIO_ALTFN_PUSH_PULL, GPIO_ALT6_SPI3);
    gpio_init_pin_af(&settings->sdi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT6_SPI3);
    gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT6_SPI3);
    gpio_init_pin_af(&settings->cs, GPIO_ALTFN_PUSH_PULL, GPIO_ALT6_SPI3);
  } else {
    gpio_init_pin_af(&settings->sdo, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->sdi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->cs, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  }

  s_spi_handles[spi].Instance = s_port[spi].base;
  s_spi_handles[spi].Init.Mode = SPI_MODE_MASTER;
  s_spi_handles[spi].Init.Direction = SPI_DIRECTION_2LINES;
  s_spi_handles[spi].Init.DataSize = SPI_DATASIZE_8BIT;

  s_spi_handles[spi].Init.NSS = SPI_NSS_SOFT;
  s_spi_handles[spi].Init.FirstBit = SPI_FIRSTBIT_MSB;
  s_spi_handles[spi].Init.TIMode = SPI_TIMODE_DISABLE;
  s_spi_handles[spi].Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  s_spi_handles[spi].Init.CRCPolynomial = 7;
  s_spi_handles[spi].Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  switch (settings->mode) {
    case SPI_MODE_0:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_LOW;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SPI_MODE_1:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_LOW;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    case SPI_MODE_2:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_HIGH;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SPI_MODE_3:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_HIGH;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  s_port[spi].rcc_cmd();

  if (HAL_SPI_Init(&s_spi_handles[spi]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_port[spi].initialized = true;
  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  if (!s_port[spi].initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (spi >= NUM_SPI_PORTS || tx_data == NULL || rx_data == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (tx_len > SPI_MAX_NUM_DATA || rx_len > SPI_MAX_NUM_DATA) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (xSemaphoreTake(s_spi_port_handle[spi], portMAX_DELAY) != pdTRUE) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  HAL_StatusTypeDef status;

  if (tx_len > 0) {
    status = HAL_SPI_Transmit_IT(&s_spi_handles[spi], tx_data, tx_len);
    if (status != HAL_OK) {
      xSemaphoreGive(s_spi_port_handle[spi]);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    if (xSemaphoreTake(s_spi_cmplt_handle[spi], portMAX_DELAY) != pdTRUE) {
      xSemaphoreGive(s_spi_port_handle[spi]);
      return STATUS_CODE_TIMEOUT;
    }
  }

  if (rx_len > 0) {
    status = HAL_SPI_Receive_IT(&s_spi_handles[spi], rx_data, rx_len);
    if (status != HAL_OK) {
      xSemaphoreGive(s_spi_port_handle[spi]);
      return STATUS_CODE_INTERNAL_ERROR;
    }

    if (xSemaphoreTake(s_spi_cmplt_handle[spi], portMAX_DELAY) != pdTRUE) {
      xSemaphoreGive(s_spi_port_handle[spi]);
      return STATUS_CODE_TIMEOUT;
    }
  }

  xSemaphoreGive(s_spi_port_handle[spi]);

  return STATUS_CODE_OK;
}
