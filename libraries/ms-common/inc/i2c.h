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

/**
 * @brief  I2C timeout in milliseconds
 */
#define I2C_TIMEOUT_MS 100

/**
 * @brief  None of our I2C transactions should need to be longer than 32 bytes
 */
#define I2C_MAX_NUM_DATA 32

/**
 * @brief I2C address type
 */
typedef uint8_t I2CAddress;

/**
 * @brief I2C port type
 */
typedef enum {
  I2C_PORT_1 = 0,
  I2C_PORT_2,
  NUM_I2C_PORTS,
} I2CPort;

/**
 * @brief I2C speed type
 */
typedef enum {
  I2C_SPEED_STANDARD = 0,  // 100kHz
  I2C_SPEED_FAST,          // 400 kHz
  NUM_I2C_SPEEDS,
} I2CSpeed;

/**
 * @brief I2C settings struct
 */
typedef struct {
  I2CSpeed speed;
  GpioAddress sda;
  GpioAddress scl;
} I2CSettings;

/**
 * @brief   Initialize the selected I2C peripheral
 *
 * @param   i2c       I2C port to initialize. Must be one of the values from the I2CPort enum.
 * @param   settings  Pointer to the I2CSettings struct.
 *
 * @return  STATUS_CODE_OK if the initialization was successful.
 *          STATUS_CODE_INVALID_ARGS if given invalid arguments.
 *          STATUS_CODE_INTERNAL_ERROR if initialization failed.
 */
StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings);

/**
 * @brief   Reads |rx_len| data from specified address at port
 *
 * @param   i2c       The I2C port to use.
 * @param   addr      The I2C address to read from.
 * @param   rx_data   Pointer to where the received data will be stored.
 * @param   rx_len    The number of bytes to read.
 *
 * @return  STATUS_CODE_TIMEOUT if I2C_TIMEOUT_MS is exceeded.
 *          STATUS_CODE_INTERNAL_ERROR if bus issue has occurred, transaction should be retried.
 */
StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len);

/**
 * @brief   Writes |tx_len| data to specified address at port
 *
 * @param   i2c
 * @param   addr
 * @param   tx_data
 * @param   tx_len
 *
 * @return  STATUS_CODE_TIMEOUT if I2C_MUTEX_WAIT_MS is exceeded.
 *          STATUS_CODE_INTERNAL_ERROR if bus issue has occurred, transaction should be retried
 */
StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len);

/**
 * @brief   Reads from register address specified by reg.
 */
StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len);

/**
 * @brief   Writes to register address specified by reg
 */
StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len);

#ifdef MS_PLATFORM_X86

/**
 * @brief   Used for tests only. Sets given data in the rx queue.
 */
StatusCode i2c_set_data(I2CPort i2c, uint8_t *tx_data, size_t tx_len);

/**
 * @brief   Used for tests only. Reads data from the tx_queue.
 */
StatusCode i2c_get_data(I2CPort i2c, uint8_t *rx_data, size_t rx_len);
#endif
