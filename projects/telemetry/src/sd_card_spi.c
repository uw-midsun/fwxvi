#include "sd_card_spi.h"

#include "FreeRTOS.h"
#include "spi.h"
#include "task.h"

static SpiSettings s_settings[NUM_SD_SPI_PORTS];
static bool s_initialized[NUM_SD_SPI_PORTS];
static TaskHandle_t s_selected[NUM_SD_SPI_PORTS];

static const SpiPort s_ports[] = { SPI_PORT_1, SPI_PORT_2, SPI_PORT_3 };
static const SpiBaudrate s_bauds[] = { SPI_BAUDRATE_312_5KHZ, SPI_BAUDRATE_625KHZ, SPI_BAUDRATE_1_25MHZ, SPI_BAUDRATE_2_5MHZ, SPI_BAUDRATE_5MHZ, SPI_BAUDRATE_10MHZ, SPI_BAUDRATE_20MHZ, SPI_BAUDRATE_40MHZ };
static const SpiMode s_modes[] = { SPI_MODE_0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3 };

StatusCode sd_spi_init(SdSpiPort spi, const SdSpiSettings *settings) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || !settings || (unsigned)settings->baudrate >= NUM_SD_SPI_BAUDRATES || (unsigned)settings->mode >= NUM_SD_SPI_MODES) return STATUS_CODE_INVALID_ARGS;
  if (s_initialized[spi]) return STATUS_CODE_ALREADY_INITIALIZED;
  s_settings[spi] = (SpiSettings){ .baudrate = s_bauds[settings->baudrate], .mode = s_modes[settings->mode], .sdo = settings->mosi, .sdi = settings->miso, .sclk = settings->sclk, .cs = settings->cs };
  StatusCode result = spi_init(s_ports[spi], &s_settings[spi]);
  if (result == STATUS_CODE_RESOURCE_EXHAUSTED || result == STATUS_CODE_ALREADY_INITIALIZED) result = spi_register_device(s_ports[spi], &s_settings[spi]);
  if (result == STATUS_CODE_OK) s_initialized[spi] = true;
  return result;
}

StatusCode sd_spi_tx(SdSpiPort spi, uint8_t *data, size_t length) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || !data || !length) return STATUS_CODE_INVALID_ARGS;
  if (!s_initialized[spi]) return STATUS_CODE_UNINITIALIZED;
  if (!s_selected[spi]) {
    /* Only all-ones startup clocks are valid while deselected. */
    for (size_t i = 0; i < length; ++i)
      if (data[i] != 0xFFU) return STATUS_CODE_INVALID_ARGS;
    return spi_deselected_clocks(s_ports[spi], &s_settings[spi], length);
  }
  return spi_transaction_transfer(s_ports[spi], data, NULL, length, 0xFFU, 100U);
}

StatusCode sd_spi_rx(SdSpiPort spi, uint8_t *data, size_t length, uint8_t filler) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || !data || !length) return STATUS_CODE_INVALID_ARGS;
  if (!s_initialized[spi] || !s_selected[spi]) return STATUS_CODE_UNINITIALIZED;
  return spi_transaction_transfer(s_ports[spi], NULL, data, length, filler, 100U);
}

StatusCode sd_spi_exchange(SdSpiPort spi, uint8_t *tx, size_t tx_len, uint8_t *rx, size_t rx_len) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || (!tx_len && !rx_len) || (tx_len && !tx) || (rx_len && !rx)) return STATUS_CODE_INVALID_ARGS;
  if (!s_initialized[spi] || !s_selected[spi]) return STATUS_CODE_UNINITIALIZED;
  size_t length = tx_len > rx_len ? tx_len : rx_len;
  for (size_t i = 0; i < length; ++i) {
    uint8_t output = i < tx_len ? tx[i] : 0xFFU;
    status_ok_or_return(spi_transaction_transfer(s_ports[spi], &output, i < rx_len ? &rx[i] : NULL, 1U, 0xFFU, 100U));
  }
  return STATUS_CODE_OK;
}

StatusCode sd_spi_cs_set_state(SdSpiPort spi, GpioState state) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS) return STATUS_CODE_INVALID_ARGS;
  if (!s_initialized[spi]) return STATUS_CODE_UNINITIALIZED;
  if (state == GPIO_STATE_LOW) {
    if (s_selected[spi]) return STATUS_CODE_RESOURCE_EXHAUSTED;
    StatusCode result = spi_transaction_begin(s_ports[spi], &s_settings[spi]);
    if (result == STATUS_CODE_OK) s_selected[spi] = xTaskGetCurrentTaskHandle();
    return result;
  }
  if (!s_selected[spi]) return STATUS_CODE_OK;
  if (s_selected[spi] != xTaskGetCurrentTaskHandle()) return STATUS_CODE_RESOURCE_EXHAUSTED;
  StatusCode result = spi_transaction_end(s_ports[spi], true);
  s_selected[spi] = NULL;
  return result;
}

GpioState sd_spi_cs_get_state(SdSpiPort spi) {
  return (unsigned)spi < NUM_SD_SPI_PORTS && s_selected[spi] ? GPIO_STATE_LOW : GPIO_STATE_HIGH;
}

StatusCode sd_spi_set_frequency(SdSpiPort spi, SdSpiBaudrate baudrate) {
  if ((unsigned)spi >= NUM_SD_SPI_PORTS || (unsigned)baudrate >= NUM_SD_SPI_BAUDRATES) return STATUS_CODE_INVALID_ARGS;
  if (!s_initialized[spi]) return STATUS_CODE_UNINITIALIZED;
  if (s_selected[spi]) return STATUS_CODE_RESOURCE_EXHAUSTED;
  s_settings[spi].baudrate = s_bauds[baudrate];
  return STATUS_CODE_OK; /* Applied by the bus owner at next begin/clocks. */
}
