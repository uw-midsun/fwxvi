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

static const uint16_t s_spi_baudrate_map[] = {
  [SPI_BAUDRATE_312_5KHZ] = SPI_BAUDRATEPRESCALER_256, [SPI_BAUDRATE_625KHZ] = SPI_BAUDRATEPRESCALER_128, [SPI_BAUDRATE_1_25MHZ] = SPI_BAUDRATEPRESCALER_64,
  [SPI_BAUDRATE_2_5MHZ] = SPI_BAUDRATEPRESCALER_32,    [SPI_BAUDRATE_5MHZ] = SPI_BAUDRATEPRESCALER_16,    [SPI_BAUDRATE_10MHZ] = SPI_BAUDRATEPRESCALER_8,
  [SPI_BAUDRATE_20MHZ] = SPI_BAUDRATEPRESCALER_4,      [SPI_BAUDRATE_40MHZ] = SPI_BAUDRATEPRESCALER_2,
};

static SPI_HandleTypeDef s_spi_handles[NUM_SPI_PORTS];
static GpioAddress s_spi_cs_handles[NUM_SPI_PORTS];

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

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  GPIO_TypeDef *gpio_port;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (hspi->Instance == SPI1) {
    gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (s_spi_cs_handles[SPI_PORT_1].port * GPIO_ADDRESS_OFFSET));
    HAL_GPIO_WritePin(gpio_port, (1U << (s_spi_cs_handles[SPI_PORT_1].pin)), 1U);
  } else if (hspi->Instance == SPI2) {
    gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (s_spi_cs_handles[SPI_PORT_2].port * GPIO_ADDRESS_OFFSET));
    HAL_GPIO_WritePin(gpio_port, (1U << (s_spi_cs_handles[SPI_PORT_2].pin)), 1U);
  } else {
    gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (s_spi_cs_handles[SPI_PORT_3].port * GPIO_ADDRESS_OFFSET));
    HAL_GPIO_WritePin(gpio_port, (1U << (s_spi_cs_handles[SPI_PORT_3].pin)), 1U);
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
    gpio_init_pin_af(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_ALT6_SPI3);
  } else {
    gpio_init_pin_af(&settings->sdo, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->sdi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_ALT5_SPI1);
  }

  s_spi_cs_handles[spi] = settings->cs;
  gpio_set_state(&s_spi_cs_handles[spi], GPIO_STATE_HIGH);

  s_spi_handles[spi].Instance = s_port[spi].base;
  s_spi_handles[spi].Init.Mode = SPI_MODE_MASTER;
  s_spi_handles[spi].Init.Direction = SPI_DIRECTION_2LINES;
  s_spi_handles[spi].Init.DataSize = SPI_DATASIZE_8BIT;
  s_spi_handles[spi].Init.NSS = SPI_NSS_SOFT;
  s_spi_handles[spi].Init.BaudRatePrescaler = s_spi_baudrate_map[settings->baudrate];
  s_spi_handles[spi].Init.FirstBit = SPI_FIRSTBIT_MSB;
  s_spi_handles[spi].Init.TIMode = SPI_TIMODE_DISABLE;
  s_spi_handles[spi].Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  s_spi_handles[spi].Init.CRCPolynomial = 0U; /* CRC Not used */
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

  interrupt_nvic_enable(s_port[spi].irq, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(s_port[spi].irq, INTERRUPT_PRIORITY_HIGH);

  s_port[spi].initialized = true;
  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  if (!s_port[spi].initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (spi >= NUM_SPI_PORTS || (tx_len == 0U && rx_len == 0U)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (tx_len > SPI_MAX_NUM_DATA || rx_len > SPI_MAX_NUM_DATA) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (xSemaphoreTake(s_spi_port_handle[spi], portMAX_DELAY) != pdTRUE) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  HAL_StatusTypeDef status;

  gpio_set_state(&s_spi_cs_handles[spi], GPIO_STATE_LOW);

  if (tx_len > 0) {
    /* Polling transmit to reduce overhead from semaphores/synchronization */
    status = HAL_SPI_Transmit(&s_spi_handles[spi], tx_data, tx_len, HAL_MAX_DELAY);
    if (status != HAL_OK) {
      gpio_set_state(&s_spi_cs_handles[spi], GPIO_STATE_HIGH);
      xSemaphoreGive(s_spi_port_handle[spi]);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }

  if (rx_len > 0) {
    /* Interrupt based receive to continue running thread and automatically pull CS high */
    status = HAL_SPI_Receive_IT(&s_spi_handles[spi], rx_data, rx_len);
    if (status != HAL_OK) {
      gpio_set_state(&s_spi_cs_handles[spi], GPIO_STATE_HIGH);
      xSemaphoreGive(s_spi_port_handle[spi]);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }

  xSemaphoreGive(s_spi_port_handle[spi]);

  return STATUS_CODE_OK;
}
