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

/* Callback functions for HAL SPI TX */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {}

/* Callback functions for HAL SPI RX */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {}

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (spi > NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_port[spi].initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  // creates mutex
  // mechanism used to prevent multiple tasks from accessing same thing
  // s_spi_port_handle holds handle to mutex
  // &s_spi_port_mutex -> points to predefined structure
  s_spi_port_handle[spi] = xSemaphoreCreateMutexStatic(&s_spi_port_mutex[spi]);
  
  // creates binary semaphore
  // used to signal another task that some action is complete
  // same format as above
  s_spi_cmplt_handle[spi] = xSemaphoreCreateBinaryStatic(&s_spi_cmplt_sem[spi]);

  if (s_spi_port_handle[spi] == NULL || s_spi_cmplt_handle[spi] == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin_af(&settings->sdo, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin_af(&settings->sdi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);

  s_spi_handles[spi].Instance = s_port[spi].base;
  s_spi_handles[spi].Init.Mode = settings->mode;
  s_spi_handles[spi].Init.Direction = SPI_DIRECTION_1LINE;
  s_spi_handles[spi].Init.DataSize = SPI_DATASIZE_8BIT; //?
  s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_LOW;
  s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_1EDGE;
  s_spi_handles[spi].Init.NSS = SPI_NSS_SOFT;
  s_spi_handles[spi].Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  s_spi_handles[spi].Init.FirstBit = SPI_FIRSTBIT_MSB;
  s_spi_handles[spi].Init.TIMode = SPI_TIMODE_DISABLE;
  s_spi_handles[spi].Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  s_spi_handles[spi].Init.CRCLength = SPI_CRC_LENGTH_8BIT;
  s_spi_handles[spi].Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  interrupt_nvic_enable(s_port[spi].irq, INTERRUPT_PRIORITY_HIGH);

  s_port[spi].rcc_cmd();

  if (HAL_SPI_Init(&s_spi_handles[spi]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_port[spi].initialized = true;

  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  if (tx_data == NULL || spi >= NUM_SPI_PORTS || rx_len > SPI_MAX_NUM_DATA) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_port[spi].initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (xSemaphoreTake(s_spi_port_handle[spi], pdMS_TO_TICKS(SPI_TIMEOUT_MS) != pdTRUE)) {
    return STATUS_CODE_TIMEOUT;
  }

  HAL_StatusTypeDef tx_status;
  HAL_StatusTypeDef rx_status;
  spi_set_cs(spi, false);
  tx_status = HAL_SPI_Transmit_IT(&s_spi_handles[spi], tx_data, tx_len);
  rx_status = HAL_SPI_Receive_IT(&s_spi_handles[spi], rx_data, rx_len);
  spi_set_cs(spi, true);

  if (tx_status != HAL_OK || rx_status != HAL_OK) {
    xSemaphoreGive(s_spi_port_handle[spi]);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if(xSemaphoreTake(s_spi_cmplt_handle[spi], pdMS_TO_TICKS(SPI_TIMEOUT_MS) != pdTRUE)) {
    xSemaphoreGive(s_spi_port_handle[spi]);
    return STATUS_CODE_TIMEOUT;
  }

  xSemaphoreGive(s_spi_port_handle[spi]);

  return STATUS_CODE_OK; 
}

StatusCode spi_get_tx_data(SpiPort spi, uint8_t *data, uint8_t len) {
  return spi_exchange(spi, data, len, NULL, 0);
}

StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len) {
  return spi_exchange(spi, NULL, 0, data, len);
}
