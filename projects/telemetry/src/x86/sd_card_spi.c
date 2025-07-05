/************************************************************************************************
 * @file    sd_card_spi.c
 *
 * @brief   Sd Card Spi
 *
 * @date    2025-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */
#include "sd_card_spi.h"

StatusCode sd_spi_init(SdSpiPort spi, const SdSpiSettings *settings) {
  return STATUS_CODE_OK;
}

StatusCode sd_spi_tx(SdSpiPort spi, uint8_t *tx_data, size_t tx_len) {
  return STATUS_CODE_OK;
}

StatusCode sd_spi_rx(SdSpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  return STATUS_CODE_OK;
}

StatusCode sd_spi_exchange(SdSpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  return STATUS_CODE_OK;
}

StatusCode sd_spi_cs_set_state(SdSpiPort spi, GpioState state) {
  return STATUS_CODE_OK;
}

StatusCode sd_spi_cs_get_state(SdSpiPort spi, GpioState *input_state) {
  return STATUS_CODE_OK;
}

StatusCode sd_spi_set_frequency(SdSpiPort spi, SdSpiBaudrate baudrate) {
  return STATUS_CODE_OK;
}
