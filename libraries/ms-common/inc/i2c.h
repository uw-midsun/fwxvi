#pragma once

/************************************************************************************************
 * i2c.h
 *
 * I2C Library Header file
 *
 * Created: 2024-11-09
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "status.h"

#define I2C_TIMEOUT_MS 100

// None of our I2C transactions should need to be longer than 32 bytes
#define I2C_MAX_NUM_DATA 32

typedef uint8_t I2CAddress;

typedef enum {
  I2C_PORT_1 = 0,
  I2C_PORT_2,
  NUM_I2C_PORTS,
} I2CPort;

typedef enum {
  I2C_SPEED_STANDARD = 0,  // 100kHz
  I2C_SPEED_FAST,          // 400 kHz
  NUM_I2C_SPEEDS,
} I2CSpeed;

typedef struct {
  I2CSpeed speed;
  GpioAddress sda;
  GpioAddress scl;
} I2CSettings;

// Initializes selected I2C peripheral
StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings);

// Reads |rx_len| data from specified address at port
// Returns:
// STATUS_CODE_TIMEOUT - if I2C_TIMEOUT_MS is exceeded
// STATUS_CODE_INTERNAL_ERROR - bus issue has occurred, transaction should be retried
StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len);

// Writes |tx_len| data to specified address at port
// Returns:
// STATUS_CODE_TIMEOUT - if I2C_MUTEX_WAIT_MS is exceeded in transaction
// STATUS_CODE_INTERNAL_ERROR - bus issue has occurred, transaction should be retried
StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len);

// Reads from register address specified by reg
StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len);

// Writes to register address specified by reg
StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len);
                         

#ifdef MS_PLATFORM_X86
// used for tests only. Sets given data in the rx queue.
StatusCode i2c_set_data(I2CPort i2c, uint8_t *tx_data, size_t tx_len);

// used for tests only. Reads data from the tx_queue.
StatusCode i2c_get_data(I2CPort i2c, uint8_t *rx_data, size_t rx_len);
#endif
