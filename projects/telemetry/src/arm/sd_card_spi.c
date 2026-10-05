/************************************************************************************************
 * @file    sd_card_spi.c
 *
 * @brief   SD Card SPI Library for STM32L4
 *
 * @date    2025-07-05
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_spi.h"

/* Intra-component Headers */
#include "sd_card_spi.h"

//! Left the driver generic to make it easier to port

#define SD_SPI_TIMEOUT_MS 100U
#define SD_SPI_TRANSFER_SIZE 64U

static inline void s_enable_spi1(void) {
  __HAL_RCC_SPI1_CLK_ENABLE();
}

static inline void s_enable_spi2(void) {
  __HAL_RCC_SPI2_CLK_ENABLE();
}

static inline void s_enable_spi3(void) {
  __HAL_RCC_SPI3_CLK_ENABLE();
}

/** @brief SD SPI Port data */
typedef struct {
  SPI_TypeDef *base;
  void (*rcc_cmd)(void);
  bool initialized;
  GpioAddress cs;
} SdSpiPortData;

static SdSpiPortData s_port[NUM_SD_SPI_PORTS] = {
  [SD_SPI_PORT_1] = { .rcc_cmd = s_enable_spi1, .base = SPI1 },
  [SD_SPI_PORT_2] = { .rcc_cmd = s_enable_spi2, .base = SPI2 },
  [SD_SPI_PORT_3] = { .rcc_cmd = s_enable_spi3, .base = SPI3 },
};

static const uint32_t s_spi_baudrate_map[] = {
  [SD_SPI_BAUDRATE_312_5KHZ] = SPI_BAUDRATEPRESCALER_256, [SD_SPI_BAUDRATE_625KHZ] = SPI_BAUDRATEPRESCALER_128, [SD_SPI_BAUDRATE_1_25MHZ] = SPI_BAUDRATEPRESCALER_64,
  [SD_SPI_BAUDRATE_2_5MHZ] = SPI_BAUDRATEPRESCALER_32,    [SD_SPI_BAUDRATE_5MHZ] = SPI_BAUDRATEPRESCALER_16,    [SD_SPI_BAUDRATE_10MHZ] = SPI_BAUDRATEPRESCALER_8,
  [SD_SPI_BAUDRATE_20MHZ] = SPI_BAUDRATEPRESCALER_4,      [SD_SPI_BAUDRATE_40MHZ] = SPI_BAUDRATEPRESCALER_2,
};

static SPI_HandleTypeDef s_spi_handles[NUM_SD_SPI_PORTS];

StatusCode sd_spi_init(SdSpiPort spi, const SdSpiSettings *settings) {
  if (settings == NULL || (unsigned)spi >= NUM_SD_SPI_PORTS || (unsigned)settings->mode >= NUM_SD_SPI_MODES || (unsigned)settings->baudrate >= NUM_SD_SPI_BAUDRATES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_port[spi].initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  GpioAlternateFunctions alternate_function = GPIO_ALT5_SPI1;
  if (spi == SD_SPI_PORT_2) {
    alternate_function = GPIO_ALT5_SPI2;
  } else if (spi == SD_SPI_PORT_3) {
    alternate_function = GPIO_ALT6_SPI3;
  }

  status_ok_or_return(gpio_init_pin_af(&settings->miso, GPIO_ALTFN_PUSH_PULL, alternate_function));
  status_ok_or_return(gpio_init_pin_af(&settings->mosi, GPIO_ALTFN_PUSH_PULL, alternate_function));
  status_ok_or_return(gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, alternate_function));
  status_ok_or_return(gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH));
  s_port[spi].cs = settings->cs;

  s_port[spi].rcc_cmd();

  s_spi_handles[spi].Instance = s_port[spi].base;
  s_spi_handles[spi].Init.Mode = SPI_MODE_MASTER;
  s_spi_handles[spi].Init.Direction = SPI_DIRECTION_2LINES;
  s_spi_handles[spi].Init.DataSize = SPI_DATASIZE_8BIT;
  s_spi_handles[spi].Init.NSS = SPI_NSS_SOFT;
  s_spi_handles[spi].Init.BaudRatePrescaler = s_spi_baudrate_map[settings->baudrate];
  s_spi_handles[spi].Init.FirstBit = SPI_FIRSTBIT_MSB;
  s_spi_handles[spi].Init.TIMode = SPI_TIMODE_DISABLE;
  s_spi_handles[spi].Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  s_spi_handles[spi].Init.CRCPolynomial = 7U;
  s_spi_handles[spi].Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  switch (settings->mode) {
    case SD_SPI_MODE_0:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_LOW;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SD_SPI_MODE_1:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_LOW;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    case SD_SPI_MODE_2:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_HIGH;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SD_SPI_MODE_3:
      s_spi_handles[spi].Init.CLKPolarity = SPI_POLARITY_HIGH;
      s_spi_handles[spi].Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  if (HAL_SPI_Init(&s_spi_handles[spi]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_port[spi].initialized = true;
  return STATUS_CODE_OK;
}

/** @brief   Transfer data without changing chip select */
static StatusCode s_transfer(SdSpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || (tx_len == 0U && rx_len == 0U) || (tx_len > 0U && tx_data == NULL) || (rx_len > 0U && rx_data == NULL)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_port[spi].initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  uint8_t tx_buffer[SD_SPI_TRANSFER_SIZE];
  uint8_t rx_buffer[SD_SPI_TRANSFER_SIZE];
  size_t length = tx_len > rx_len ? tx_len : rx_len;
  uint32_t start = HAL_GetTick();

  for (size_t offset = 0U; offset < length;) {
    size_t chunk = length - offset;
    if (chunk > sizeof(tx_buffer)) {
      chunk = sizeof(tx_buffer);
    }

    memset(tx_buffer, placeholder, chunk);
    if (offset < tx_len) {
      size_t tx_count = tx_len - offset;
      if (tx_count > chunk) {
        tx_count = chunk;
      }
      memcpy(tx_buffer, &tx_data[offset], tx_count);
    }

    uint32_t elapsed = HAL_GetTick() - start;
    if (elapsed >= SD_SPI_TIMEOUT_MS) {
      HAL_SPI_Abort(&s_spi_handles[spi]);
      return STATUS_CODE_TIMEOUT;
    }

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&s_spi_handles[spi], tx_buffer, rx_buffer, chunk, SD_SPI_TIMEOUT_MS - elapsed);
    if (status != HAL_OK) {
      HAL_SPI_Abort(&s_spi_handles[spi]);
      return status == HAL_TIMEOUT ? STATUS_CODE_TIMEOUT : STATUS_CODE_INTERNAL_ERROR;
    }

    if (offset < rx_len) {
      size_t rx_count = rx_len - offset;
      if (rx_count > chunk) {
        rx_count = chunk;
      }
      memcpy(&rx_data[offset], rx_buffer, rx_count);
    }
    offset += chunk;
  }

  return STATUS_CODE_OK;
}

StatusCode sd_spi_tx(SdSpiPort spi, uint8_t *tx_data, size_t tx_len) {
  return s_transfer(spi, tx_data, tx_len, NULL, 0U, 0xFFU);
}

StatusCode sd_spi_rx(SdSpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  return s_transfer(spi, NULL, 0U, rx_data, rx_len, placeholder);
}

StatusCode sd_spi_exchange(SdSpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  return s_transfer(spi, tx_data, tx_len, rx_data, rx_len, 0xFFU);
}

StatusCode sd_spi_cs_set_state(SdSpiPort spi, GpioState state) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || (state != GPIO_STATE_LOW && state != GPIO_STATE_HIGH)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_port[spi].initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  GpioState previous_state = gpio_get_state(&s_port[spi].cs);
  status_ok_or_return(gpio_set_state(&s_port[spi].cs, state));

  /* give 8 more clocks  */
  if (state == GPIO_STATE_HIGH && previous_state == GPIO_STATE_LOW) {
    uint8_t dummy = 0xFFU;
    return sd_spi_tx(spi, &dummy, sizeof(dummy));
  }

  return STATUS_CODE_OK;
}

GpioState sd_spi_cs_get_state(SdSpiPort spi) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || !s_port[spi].initialized) {
    return GPIO_STATE_HIGH;
  }

  return gpio_get_state(&s_port[spi].cs);
}

StatusCode sd_spi_set_frequency(SdSpiPort spi, SdSpiBaudrate baudrate) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || (unsigned)baudrate >= NUM_SD_SPI_BAUDRATES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_port[spi].initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (sd_spi_cs_get_state(spi) == GPIO_STATE_LOW) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  SPI_HandleTypeDef *handle = &s_spi_handles[spi];
  __HAL_SPI_DISABLE(handle);
  handle->Init.BaudRatePrescaler = s_spi_baudrate_map[baudrate];
  MODIFY_REG(handle->Instance->CR1, SPI_CR1_BR, handle->Init.BaudRatePrescaler);
  __HAL_SPI_ENABLE(handle);

  return STATUS_CODE_OK;
}
