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
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_spi.h"
#include "task.h"

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
#define SPI_DEVICE_LIMIT 4U
#define SPI_TRANSFER_TIMEOUT_MS 100U
#define SPI_LOCK_TIMEOUT_MS 1000U
static SpiSettings s_defaults[NUM_SPI_PORTS];
static GpioAddress s_devices[NUM_SPI_PORTS][SPI_DEVICE_LIMIT];
static size_t s_device_count[NUM_SPI_PORTS];
static TaskHandle_t s_owner[NUM_SPI_PORTS];
static GpioAddress s_selected[NUM_SPI_PORTS];

/* Mutex for port access */
static StaticSemaphore_t s_spi_port_mutex[NUM_SPI_PORTS];
static SemaphoreHandle_t s_spi_port_handle[NUM_SPI_PORTS];

void SPI1_IRQHandler(void) {
  HAL_SPI_IRQHandler(&s_spi_handles[SPI_PORT_1]);
}

void SPI2_IRQHandler(void) {
  HAL_SPI_IRQHandler(&s_spi_handles[SPI_PORT_2]);
}

void SPI3_IRQHandler(void) {
  HAL_SPI_IRQHandler(&s_spi_handles[SPI_PORT_3]);
}

/* Transactions use bounded polling transfers. No completion callback changes
 * CS or ownership; both legacy and explicit callers finish in task context. */

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if ((unsigned)spi >= NUM_SPI_PORTS || (unsigned)settings->mode >= NUM_SPI_MODES || (unsigned)settings->baudrate >= NUM_SPI_BAUDRATE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_port[spi].initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  s_spi_port_handle[spi] = xSemaphoreCreateMutexStatic(&s_spi_port_mutex[spi]);

  if (s_spi_port_handle[spi] == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  GpioAlternateFunctions af = spi == SPI_PORT_3 ? GPIO_ALT6_SPI3 : spi == SPI_PORT_2 ? GPIO_ALT5_SPI2 : GPIO_ALT5_SPI1;
  status_ok_or_return(gpio_init_pin_af(&settings->sdo, GPIO_ALTFN_PUSH_PULL, af));
  status_ok_or_return(gpio_init_pin_af(&settings->sdi, GPIO_ALTFN_PUSH_PULL, af));
  status_ok_or_return(gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, af));
  status_ok_or_return(gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH));
  s_defaults[spi] = *settings;
  s_devices[spi][0] = settings->cs;
  s_device_count[spi] = 1U;

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

  s_port[spi].initialized = true;
  return STATUS_CODE_OK;
}

static bool s_valid_settings(SpiPort spi, const SpiSettings *settings) {
  return (unsigned)spi < NUM_SPI_PORTS && settings != NULL && (unsigned)settings->mode < NUM_SPI_MODES && (unsigned)settings->baudrate < NUM_SPI_BAUDRATE;
}

StatusCode spi_register_device(SpiPort spi, const SpiSettings *settings) {
  if (!s_valid_settings(spi, settings)) return STATUS_CODE_INVALID_ARGS;
  if (!s_port[spi].initialized) return STATUS_CODE_UNINITIALIZED;
  if (xSemaphoreTake(s_spi_port_handle[spi], pdMS_TO_TICKS(SPI_LOCK_TIMEOUT_MS)) != pdTRUE) return STATUS_CODE_TIMEOUT;
  StatusCode result = STATUS_CODE_OK;
  const SpiSettings *bus = &s_defaults[spi];
  if (settings->sdo.port != bus->sdo.port || settings->sdo.pin != bus->sdo.pin || settings->sdi.port != bus->sdi.port || settings->sdi.pin != bus->sdi.pin || settings->sclk.port != bus->sclk.port ||
      settings->sclk.pin != bus->sclk.pin) {
    result = STATUS_CODE_INVALID_ARGS;
  } else {
    size_t i;
    for (i = 0; i < s_device_count[spi]; ++i) {
      if (s_devices[spi][i].port == settings->cs.port && s_devices[spi][i].pin == settings->cs.pin) break;
    }
    if (i == s_device_count[spi]) {
      if (i == SPI_DEVICE_LIMIT)
        result = STATUS_CODE_RESOURCE_EXHAUSTED;
      else {
        result = gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
        if (result == STATUS_CODE_OK) s_devices[spi][s_device_count[spi]++] = settings->cs;
      }
    }
  }
  xSemaphoreGive(s_spi_port_handle[spi]);
  return result;
}

static StatusCode s_acquire(SpiPort spi, const SpiSettings *settings, bool select) {
  if (!s_valid_settings(spi, settings)) return STATUS_CODE_INVALID_ARGS;
  if (!s_port[spi].initialized) return STATUS_CODE_UNINITIALIZED;
  size_t i;
  for (i = 0; i < s_device_count[spi]; ++i) {
    if (s_devices[spi][i].port == settings->cs.port && s_devices[spi][i].pin == settings->cs.pin) break;
  }
  if (i == s_device_count[spi]) return STATUS_CODE_INVALID_ARGS;
  if (xSemaphoreTake(s_spi_port_handle[spi], pdMS_TO_TICKS(SPI_LOCK_TIMEOUT_MS)) != pdTRUE) return STATUS_CODE_TIMEOUT;
  StatusCode result = STATUS_CODE_OK;
  for (i = 0; i < s_device_count[spi]; ++i) {
    result = gpio_set_state(&s_devices[spi][i], GPIO_STATE_HIGH);
    if (result != STATUS_CODE_OK) goto fail;
  }
  SPI_HandleTypeDef *handle = &s_spi_handles[spi];
  __HAL_SPI_DISABLE(handle);
  handle->Init.BaudRatePrescaler = s_spi_baudrate_map[settings->baudrate];
  handle->Init.CLKPolarity = settings->mode >= SPI_MODE_2 ? SPI_POLARITY_HIGH : SPI_POLARITY_LOW;
  handle->Init.CLKPhase = (settings->mode == SPI_MODE_1 || settings->mode == SPI_MODE_3) ? SPI_PHASE_2EDGE : SPI_PHASE_1EDGE;
  if (HAL_SPI_Init(handle) != HAL_OK) {
    result = STATUS_CODE_INTERNAL_ERROR;
    goto fail;
  }
  s_selected[spi] = settings->cs;
  if (select) {
    result = gpio_set_state(&settings->cs, GPIO_STATE_LOW);
    if (result != STATUS_CODE_OK) goto fail;
  }
  s_owner[spi] = xTaskGetCurrentTaskHandle();
  return STATUS_CODE_OK;
fail:
  xSemaphoreGive(s_spi_port_handle[spi]);
  return result;
}

StatusCode spi_transaction_begin(SpiPort spi, const SpiSettings *settings) {
  return s_acquire(spi, settings, true);
}

StatusCode spi_transaction_transfer(SpiPort spi, const uint8_t *tx, uint8_t *rx, size_t length, uint8_t filler, uint32_t timeout_ms) {
  if ((unsigned)spi >= NUM_SPI_PORTS || length == 0U || timeout_ms == 0U) return STATUS_CODE_INVALID_ARGS;
  if (s_owner[spi] == NULL || s_owner[spi] != xTaskGetCurrentTaskHandle()) return STATUS_CODE_UNINITIALIZED;
  uint32_t start = HAL_GetTick();
  for (size_t i = 0; i < length; ++i) {
    uint32_t elapsed = HAL_GetTick() - start;
    if (elapsed >= timeout_ms) {
      HAL_SPI_Abort(&s_spi_handles[spi]);
      return STATUS_CODE_TIMEOUT;
    }
    uint8_t output = tx ? tx[i] : filler, input;
    HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(&s_spi_handles[spi], &output, &input, 1U, timeout_ms - elapsed);
    if (result != HAL_OK) {
      HAL_SPI_Abort(&s_spi_handles[spi]);
      return result == HAL_TIMEOUT ? STATUS_CODE_TIMEOUT : STATUS_CODE_INTERNAL_ERROR;
    }
    if (rx) rx[i] = input;
  }
  return STATUS_CODE_OK;
}

StatusCode spi_transaction_end(SpiPort spi, bool sd_trailing_clocks) {
  if ((unsigned)spi >= NUM_SPI_PORTS) return STATUS_CODE_INVALID_ARGS;
  if (s_owner[spi] == NULL || s_owner[spi] != xTaskGetCurrentTaskHandle()) return STATUS_CODE_UNINITIALIZED;
  StatusCode result = gpio_set_state(&s_selected[spi], GPIO_STATE_HIGH);
  if (sd_trailing_clocks && result == STATUS_CODE_OK) result = spi_transaction_transfer(spi, NULL, NULL, 1U, 0xFFU, SPI_TRANSFER_TIMEOUT_MS);
  s_owner[spi] = NULL;
  xSemaphoreGive(s_spi_port_handle[spi]);
  return result;
}

StatusCode spi_deselected_clocks(SpiPort spi, const SpiSettings *settings, size_t length) {
  status_ok_or_return(s_acquire(spi, settings, false));
  StatusCode result = spi_transaction_transfer(spi, NULL, NULL, length, 0xFFU, SPI_TRANSFER_TIMEOUT_MS);
  StatusCode end = spi_transaction_end(spi, false);
  return result != STATUS_CODE_OK ? result : end;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  if ((unsigned)spi >= NUM_SPI_PORTS || (tx_len == 0U && rx_len == 0U) || (tx_len && !tx_data) || (rx_len && !rx_data) || tx_len > SPI_MAX_NUM_DATA || rx_len > SPI_MAX_NUM_DATA)
    return STATUS_CODE_INVALID_ARGS;
  status_ok_or_return(spi_transaction_begin(spi, &s_defaults[spi]));
  StatusCode result = STATUS_CODE_OK;
  if (tx_len) result = spi_transaction_transfer(spi, tx_data, NULL, tx_len, 0U, SPI_TRANSFER_TIMEOUT_MS);
  if (result == STATUS_CODE_OK && rx_len) result = spi_transaction_transfer(spi, NULL, rx_data, rx_len, 0U, SPI_TRANSFER_TIMEOUT_MS);
  StatusCode end = spi_transaction_end(spi, false);
  return result != STATUS_CODE_OK ? result : end;
}
