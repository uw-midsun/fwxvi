/************************************************************************************************
 * @file    sd_card_spi.c
 *
 * @brief   SD Card SPI Library for STM32L4
 *
 * @date    2025-07-05
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_spi.h"

/* Intra-component Headers */
#include "sd_card_spi.h"

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
  uint8_t irq;
  bool initialized;
  GpioAddress cs;
} SdSpiPortData;

static SdSpiPortData s_port[NUM_SD_SPI_PORTS] = {
  [SD_SPI_PORT_1] = { .rcc_cmd = s_enable_spi1, .base = SPI1, .irq = SPI1_IRQn },
  [SD_SPI_PORT_2] = { .rcc_cmd = s_enable_spi2, .base = SPI2, .irq = SPI2_IRQn },
  [SD_SPI_PORT_3] = { .rcc_cmd = s_enable_spi3, .base = SPI3, .irq = SPI3_IRQn },
};

static const uint32_t s_spi_baudrate_map[] = {
  [SD_SPI_BAUDRATE_312_5KHZ] = SPI_BAUDRATEPRESCALER_256, [SD_SPI_BAUDRATE_625KHZ] = SPI_BAUDRATEPRESCALER_128, [SD_SPI_BAUDRATE_1_25MHZ] = SPI_BAUDRATEPRESCALER_64,
  [SD_SPI_BAUDRATE_2_5MHZ] = SPI_BAUDRATEPRESCALER_32,    [SD_SPI_BAUDRATE_5MHZ] = SPI_BAUDRATEPRESCALER_16,    [SD_SPI_BAUDRATE_10MHZ] = SPI_BAUDRATEPRESCALER_8,
  [SD_SPI_BAUDRATE_20MHZ] = SPI_BAUDRATEPRESCALER_4,      [SD_SPI_BAUDRATE_40MHZ] = SPI_BAUDRATEPRESCALER_2,
};

static SPI_HandleTypeDef s_spi_handles[NUM_SD_SPI_PORTS];

StatusCode sd_spi_init(SdSpiPort spi, const SdSpiSettings *settings) {
  if (!settings || spi >= NUM_SD_SPI_PORTS || settings->mode >= NUM_SD_SPI_MODES || settings->baudrate >= NUM_SD_SPI_BAUDRATES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_port[spi].initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  if (spi == SD_SPI_PORT_3) {
    gpio_init_pin_af(&settings->miso, GPIO_INPUT_FLOATING, GPIO_ALT6_SPI3);
    gpio_init_pin_af(&settings->mosi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT6_SPI3);
    gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT6_SPI3);
  } else {
    gpio_init_pin_af(&settings->miso, GPIO_INPUT_FLOATING, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->mosi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
    gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  }

  gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
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

StatusCode sd_spi_tx(SdSpiPort spi, uint8_t *tx_data, size_t tx_len) {
  if (!s_port[spi].initialized || spi >= NUM_SD_SPI_PORTS || !tx_data || tx_len == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  HAL_StatusTypeDef status = HAL_SPI_Transmit(&s_spi_handles[spi], tx_data, tx_len, HAL_MAX_DELAY);
  return (status == HAL_OK) ? STATUS_CODE_OK : STATUS_CODE_INTERNAL_ERROR;
}

StatusCode sd_spi_rx(SdSpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  if (!s_port[spi].initialized || spi >= NUM_SD_SPI_PORTS || !rx_data || rx_len == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  for (size_t i = 0; i < rx_len; i++) {
    uint8_t tx = placeholder;
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&s_spi_handles[spi], &tx, &rx_data[i], 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode sd_spi_exchange(SdSpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  if (!s_port[spi].initialized || spi >= NUM_SD_SPI_PORTS || (tx_len == 0 && rx_len == 0)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  HAL_StatusTypeDef status = HAL_OK;

  if (tx_len > 0 && rx_len > 0) {
    size_t len = (tx_len > rx_len) ? tx_len : rx_len;
    for (size_t i = 0; i < len; i++) {
      uint8_t tx = (i < tx_len) ? tx_data[i] : 0xFF;
      uint8_t *rx_ptr = (i < rx_len) ? &rx_data[i] : NULL;
      uint8_t dummy_rx;
      status = HAL_SPI_TransmitReceive(&s_spi_handles[spi], &tx, rx_ptr ? rx_ptr : &dummy_rx, 1, HAL_MAX_DELAY);
      if (status != HAL_OK) break;
    }
  } else if (tx_len > 0) {
    status = HAL_SPI_Transmit(&s_spi_handles[spi], tx_data, tx_len, HAL_MAX_DELAY);
  } else {
    for (size_t i = 0; i < rx_len; i++) {
      uint8_t tx = 0xFF;
      status = HAL_SPI_TransmitReceive(&s_spi_handles[spi], &tx, &rx_data[i], 1, HAL_MAX_DELAY);
      if (status != HAL_OK) break;
    }
  }

  return (status == HAL_OK) ? STATUS_CODE_OK : STATUS_CODE_INTERNAL_ERROR;
}

StatusCode sd_spi_cs_set_state(SdSpiPort spi, GpioState state) {
  if (spi >= NUM_SD_SPI_PORTS || !s_port[spi].initialized) {
    return STATUS_CODE_INVALID_ARGS;
  }
  return gpio_set_state(&s_port[spi].cs, state);
}

GpioState sd_spi_cs_get_state(SdSpiPort spi) {
  if (spi >= NUM_SD_SPI_PORTS || !s_port[spi].initialized) {
    return STATUS_CODE_INVALID_ARGS;
  }
  return gpio_get_state(&s_port[spi].cs);
}

StatusCode sd_spi_set_frequency(SdSpiPort spi, SdSpiBaudrate baudrate) {
  if (spi >= NUM_SD_SPI_PORTS || !s_port[spi].initialized || baudrate >= NUM_SD_SPI_BAUDRATES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  __HAL_SPI_DISABLE(&s_spi_handles[spi]);
  uint32_t temp_cr1 = s_spi_handles[spi].Instance->CR1;
  temp_cr1 &= ~SPI_CR1_BR;
  temp_cr1 |= s_spi_baudrate_map[baudrate];
  s_spi_handles[spi].Instance->CR1 = temp_cr1;
  __HAL_SPI_ENABLE(&s_spi_handles[spi]);

  return STATUS_CODE_OK;
}
