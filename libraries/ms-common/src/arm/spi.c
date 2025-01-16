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
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_get_tx_data(SpiPort spi, uint8_t *data, uint8_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}
