/************************************************************************************************
 * @file    tcixtma1.c
 *
 * @brief   Source file to implement the TCIXTMA1 hydrogen sensing driver
 *
 * @date    2026-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"

/* Intra-component Headers */
#include "tcixtma1.h"
#include "tcixtma1_crc16.h"
#include "tcixtma1_defs.h"

// initialize the storage object
StatusCode tcixtma1_init(TCIXTMA1Storage *storage, I2CPort i2c_port, I2CAddress i2c_address) {
  if (storage == NULL || i2c_address > 127) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->i2c_port = i2c_port;
  storage->i2c_address = i2c_address;

  tcixtma1_crc16_init_table();

  return STATUS_CODE_OK;
}

StatusCode tcixtma1_get_hydrogen_concentration(TCIXTMA1Storage *storage, float *out_hydrogen_con) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t tx_buff[7];
  tx_buff[0] = TCIXTMA1_TRIGGER_CONC_CMD;
  tx_buff[1] = TCIXTMA1_TRIGGER_CONC_CONFIG;
  tx_buff[2] = TCIXTMA1_DEFAULT_RH;
  tx_buff[3] = TCIXTMA1_DEFAULT_TEMP;
  tx_buff[4] = TCIXTMA1_DEFAULT_P;

  uint16_t tx_crc = tcixtma1_crc16_calculate(tx_buff, 5);
  tx_buff[5] = tx_crc >> 8;
  tx_buff[6] = tx_crc & 0xFF;

  // master triggers conc measurement
  StatusCode status = i2c_write(storage->i2c_port, storage->i2c_address, tx_buff, sizeof(tx_buff));
  if (status != STATUS_CODE_OK) {
    return status;
  }

  // wait for a defined processing time
  delay_ms(TCIXTMA1_CONC_TIME);

  // read the measurement results
  uint8_t rx_buf[5];
  status = i2c_read(storage->i2c_port, storage->i2c_address, rx_buf, sizeof(rx_buf));
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint8_t rx_buf_status = rx_buf[0];
  if (rx_buf_status != 0) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  uint16_t rx_calc_crc = tcixtma1_crc16_calculate(rx_buf, 3);
  uint16_t rx_buf_crc = (rx_buf[3] << 8) | rx_buf[4];
  if (rx_calc_crc != rx_buf_crc) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  int16_t raw_conc = (rx_buf[1] << 8) | rx_buf[2];
  *out_hydrogen_con = (float)raw_conc * TCIXTMA1_CONC_SCALE_FACTOR;

  return STATUS_CODE_OK;
}
