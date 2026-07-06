/************************************************************************************************
 * @file   tcixtma1.c
 *
 * @brief  x86 mock driver for the TCIXTMA1 hydrogen sensing driver
 *
 * @date   2026-07-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "tcixtma1.h"

static float s_hydrogen_concentration;

StatusCode tcixtma1_init(TCIXTMA1Storage *storage, I2CPort i2c_port, I2CAddress i2c_address) {
  if (storage == NULL || i2c_address > 127) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->i2c_port = i2c_port;
  storage->i2c_address = i2c_address;

  return STATUS_CODE_OK;
}

StatusCode tcixtma1_get_hydrogen_concentration(TCIXTMA1Storage *storage, float *out_hydrogen_con) {
  if (storage == NULL || out_hydrogen_con == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  *out_hydrogen_con = s_hydrogen_concentration;
  return STATUS_CODE_OK;
}

/* Setter for simulation/testing */
void tcixtma1_set_hydrogen_concentration(float hydrogen_con) {
  s_hydrogen_concentration = hydrogen_con;
}
