#pragma once

/************************************************************************************************
 * @file   i2c.h
 *
 * @brief  I2C Library Header file
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "status.h"

/**
 * @defgroup I2C
 * @brief    I2C library
 * @{
 */

/** @brief  Maximum time permitted for an I2C transaction */
#define I2C_TIMEOUT_MS 100U
/** @brief  None of our I2C transactions should need to be longer than 32 bytes */
#define I2C_MAX_NUM_DATA 32U

/** @brief  I2C address type */
typedef uint8_t I2CAddress;

/** @brief  I2C Port selection */
typedef enum {
  I2C_PORT_1 = 0, /**< SCL: PA9 and SDA: PA10 */
  I2C_PORT_2,     /**< SCL: PB10 and SDA: PB11 */
  NUM_I2C_PORTS,  /**< Number of I2C Ports */
} I2CPort;

/** @brief  I2C speed type */
typedef enum {
  I2C_SPEED_STANDARD = 0, /**< 100kHz */
  I2C_SPEED_FAST,         /**< 400 kHz */
  NUM_I2C_SPEEDS,         /**< Number of I2C Speeds */
} I2CSpeed;

/**
 * @brief   I2C settings struct
 * @details I2C Speed is 100kHz for standard mode and 400kHz for fast mode.
 *          SDA is an open-drain data pin. SCL is an open-drain clock pin.
 */
typedef struct {
  I2CSpeed speed;  /**< I2C Clock speed selection */
  GpioAddress sda; /**< GPIO Pin to transmit and receive data */
  GpioAddress scl; /**< GPIO Pin to provide clock signal */
} I2CSettings;

/**
 * @brief   Initialize I2C instance
 * @param   i2c Specifies which I2C port to initialize
 * @param   settings Pointer to I2C initialization settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 *          STATUS_CODE_RESOURCE_EXHAUSTED if already initialized
 */
StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings);

/**
 * @brief   Reads data using the I2C port from the specified address
 * @param   i2c Specifies which I2C port to read with
 * @param   addr The I2C address to read from
 * @param   rx_data Pointer to a buffer to receive data
 * @param   rx_len Length of data to receive
 * @return  STATUS_CODE_OK if reading succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission fails
 *          STATUS_CODE_TIMEOUT if transmission takes too long
 */
StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len);

/**
 * @brief   Writes data using the I2C port to the specified address
 * @param   i2c Specifies which I2C port to write with
 * @param   addr Specifies the I2C address to write to
 * @param   tx_data Pointer to a buffer to transmit data
 * @param   tx_len Length of the data to transmit
 * @return  STATUS_CODE_OK if writing succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission fails
 *          STATUS_CODE_TIMEOUT if transmission takes too long
 */
StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len);

/**
 * @brief   Reads data using the I2C port from the specified address and register
 * @param   i2c Specifies which I2C port to read with
 * @param   addr Specifies the I2C address to read from
 * @param   reg Specifies the register to read from
 * @param   rx_data Pointer to a buffer to receive data
 * @param   rx_len Length of the data to receive
 * @return  STATUS_CODE_OK if reading succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission fails
 *          STATUS_CODE_TIMEOUT if transmission takes too long
 */
StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len);

/**
 * @brief   Writes data using the I2C port to the specified address and register
 * @param   i2c Specifies which I2C port to write with
 * @param   addr Specifies the I2C address to write to
 * @param   reg Specifies the register to write to
 * @param   tx_data Pointer to a buffer to transmit data
 * @param   tx_len Length of the data to transmit
 * @return  STATUS_CODE_OK if writing succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission fails
 *          STATUS_CODE_TIMEOUT if transmission takes too long
 */
StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data, size_t tx_len);

#ifdef MS_PLATFORM_X86

/**
 * @brief   Sets I2C RX queue with the given data
 * @param   i2c Specifies which I2C port to update
 * @param   data Pointer to a buffer of data to set
 * @param   len Length of the data to set
 * @return  STATUS_CODE_OK if data is set successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode i2c_set_rx_data(I2CPort i2c, uint8_t *data, size_t len);

/**
 * @brief   Gets data from the I2C TX queue
 * @param   i2c Specifies which I2C port to read from
 * @param   data Pointer to a buffer of data to fill
 * @param   len Length of the data to retrieve
 * @return  STATUS_CODE_OK if data is retrieved successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode i2c_get_tx_data(I2CPort i2c, uint8_t *data, size_t len);

#endif

/** @} */
