#pragma once

/************************************************************************************************
 * @file   ltc_afe.h
 *
 * @brief  Header file for the LTC8611 AFE driver
 *
 * @date   2025-04-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/**
 * @note Requires GPIO, Interrupts, Soft Timers, and the Event Queue to be initialized before use.
 * @note All voltage units are in 100 µV.
 * @note This module supports AFEs with 12 or more cells using the cell/aux bitset.
 * @note Due to long conversion delays, the driver uses a finite state machine (FSM)
 *       to return control to the application during conversions.
 * @note This module is mostly exposed for the FSM. Do not use functions in this module directly.
 */

/* Standard library Headers */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Inter-component Headers */
#include "gpio.h"
#include "spi.h"

/* Intra-component Headers */
#include "fsm.h"
#include "ltc_afe_regs.h"
#include "status.h"

/**
 * @defgroup LTC6811
 * @brief    LTC6811 library
 * @{
 */

/* Removes padding, if possible */
#if defined(__GNUC__)
#define _PACKED __attribute__((packed))
#else
#define _PACKED
#endif

/**
 * @brief Maximum AFE devices in daisy chain config
 * @note This is an arbitrary limitation, can be increased/decreased if needed
 */
#define LTC_AFE_MAX_DEVICES 3

/**
 * @brief Maximum x of each device
 * @note This is a device limitation
 */
#define LTC_AFE_MAX_CELLS_PER_DEVICE 12
#define LTC_AFE_MAX_THERMISTORS_PER_DEVICE 8

/**
 * @brief Maximum across all devices
 */
#define LTC_AFE_MAX_CELLS (LTC_AFE_MAX_DEVICES * LTC_AFE_MAX_CELLS_PER_DEVICE)
#define LTC_AFE_MAX_THERMISTORS (LTC_AFE_MAX_DEVICES * LTC_AFE_MAX_THERMISTORS_PER_DEVICE)

/**
 * @brief   Select the ADC mode
 * @details Trade-off between speed or minimizing noise
 * @note    See p 50 for conversion times and p 23 (table 3) for noise
 */
typedef enum {
  LTC_AFE_ADC_MODE_422KHZ = 0,
  LTC_AFE_ADC_MODE_27KHZ, /**< Normal */
  LTC_AFE_ADC_MODE_7KHZ,  /**< Fast */
  LTC_AFE_ADC_MODE_26HZ,  /**< Filtered */
  LTC_AFE_ADC_MODE_1KHZ,
  LTC_AFE_ADC_MODE_14KHZ,
  LTC_AFE_ADC_MODE_3KHZ,
  LTC_AFE_ADC_MODE_2KHZ,
  NUM_LTC_AFE_ADC_MODES
} LtcAfeAdcMode;

/**
 * @brief   Afe Settings Data
 * @details Set by the user when `ltc_afe_init` is called
 *          Stores SPI information, which cell and aux inputs are enabled, and number of things
 * @note    For more info on `SpiSettings` refer to `spi.h`
 */
typedef struct LtcAfeSettings {
  LtcAfeAdcMode adc_mode; /**< Determines ADC Mode */

  uint16_t cell_bitset[LTC_AFE_MAX_DEVICES]; /**< Bitset showing cells are enabled for each device */
  uint16_t aux_bitset[LTC_AFE_MAX_DEVICES];  /**< Bitset showing aux inputs enabled for each device */

  size_t num_devices;     /**< Number of AFE devices */
  size_t num_cells;       /**< Number of TOTAL cells across all devices */
  size_t num_thermistors; /**< Number of TOTAL thermistors (aux inputs) across all devices */

  SpiSettings spi_settings; /**< SPI settings for AFE */
  const SpiPort spi_port;   /**< Determines which SPI port to use */
} LtcAfeSettings;

/**
 * @brief   Runtime Data Storage
 * @details Stores settings, configs, and voltages for all AFE devices
 * @note    Raw indices mean
 *          - Index `0` corresponds to the first cell/aux value of the first AFE device.
 *          - Index `n * max_x_per_device` corresponds to the first value of the (n+1)th AFE device.
 *          - Each AFE's data (voltages, lookups, etc.) is stored contiguously before the next AFE's data.
 */
typedef struct LtcAfeStorage {
  uint16_t cell_voltages[LTC_AFE_MAX_CELLS];      /**< Stores cell voltages for all devices */
  uint16_t aux_voltages[LTC_AFE_MAX_THERMISTORS]; /**< Stores aux voltages for all devices */

  uint16_t cell_result_lookup[LTC_AFE_MAX_CELLS];      /**< Map raw cell indices read from AFE to `cell_voltages` */
  uint16_t aux_result_lookup[LTC_AFE_MAX_THERMISTORS]; /**< Map raw aux input indices read from AFE to `aux_voltages` */
  uint16_t discharge_cell_lookup[LTC_AFE_MAX_CELLS];   /**< Map indicies of `cell_voltages` to raw cell indices */

  LtcAfeSettings *settings;                /**< Stores settings for AFE devices, set by the user */
  LtcAfeWriteConfigPacket *device_configs; /**< Stores the Configuration of each device in the CFGR register */
} LtcAfeStorage;

/**
 * @brief   Initializes the LTC AFE system with provided configuration settings
 * @details Validates user-defined configuration parameters, sets up SPI communication,
 *          calculates result buffer offsets, initializes the CRC15 lookup table,
 *          and writes configuration settings to all AFE devices
 * @param   afe Pointer to LtcAfeStorage struct, stores runtime data and settings of AFE
 * @param   config Pointer to constant LtcAfeSettings struct, contains user-specified configuration parameters
 * @return  STATUS_CODE_OK if initialization is successful
 *          STATUS_CODE_INVALID_ARGS if device, cell, or thermistor counts exceed defined maximums
 *          STATUS_CODE_INTERNAL_ERROR if SPI or configuration writes fail during setup
 */
StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *config);

/**
 * @brief   Writes configuration bits onto CFGR (Configuration Register Group)
 * @details Writes configuration bits to each of the AFE devices' configuration register bits
 * @param   afe Pointer to LtcAfeStorage struct, stores runtime data and settings of AFE
 * @return  STATUS_CODE_OK if configs were written onto register
 *          STATUS_CODE_INVALID_ARGS if one of the parameters in `spi_exchange` is incorrect in `prv_write_config`
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission/receiving fails
 *          STATUS_CODE_TIMEOUT if transmission/receiving takes too long
 * @note    Only for initial config
 */
StatusCode ltc_afe_write_config(LtcAfeStorage *afe);

/**
 * @brief   Triggers ADC conversion for all enabled cell voltage inputs
 * @details Builds and transmits the ADCV command using the configured ADC mode to start
 *          cell voltage conversion across all AFE devices. Puts device into active mode
 *          before initiating conversion.
 * @param   afe Pointer to LtcAfeStorage struct containing runtime data and settings
 * @return  STATUS_CODE_OK if the ADCV command was successfully transmitted
 *          STATUS_CODE_INVALID_ARGS if arguments passed to spi_exchange are invalid
 *          STATUS_CODE_INTERNAL_ERROR if SPI transmission fails
 */
StatusCode ltc_afe_trigger_cell_conv(LtcAfeStorage *afe);

/**
 * @brief   Triggers ADC conversion for an auxiliary (thermistor) input
 * @details Selects the specified thermistor by configuring GPIO bits, then builds and transmits
 *          the ADAX command to begin auxiliary ADC conversion using GPIO4. Supports thermistors
 *          mapped to 5 available GPIO inputs across devices.
 * @param   afe Pointer to LtcAfeStorage struct containing runtime data and settings
 * @param   thermistor Index of the thermistor to measure (0-7 across devices)
 * @return  STATUS_CODE_OK if the ADAX command was successfully transmitted
 *          STATUS_CODE_INVALID_ARGS if arguments passed to spi_exchange are invalid
 *          STATUS_CODE_INTERNAL_ERROR if SPI transmission fails
 */
StatusCode ltc_afe_trigger_aux_conv(LtcAfeStorage *afe, uint8_t device_cell);

/**
 * @brief   Reads and stores cell voltages from the voltage cell registers of each afe
 * @details RDCVx (x = A, B, C, D) command is sent to read converted voltage from respective register group
 *          Received PEC and PEC of data is compared to see if data is valid
 *          Voltages are stored in the `afe->cell_voltages` array, if cell status is enabled
 * @param   afe Pointer to LtcAfeStorage struct, stores runtime data and settings of AFE
 * @return  STATUS_CODE_OK if cells were read correctly
 *          STATUS_CODE_INTERNAL_ERROR if read PEC and calcualted PEC do not match up
 *          STATUS_CODE_INVALID_ARGS if arguments are invalid in `prv_read_register`
 */
StatusCode ltc_afe_read_cells(LtcAfeStorage *afe);

/**
 * @brief   Reads and stores auxillary input from GPIO4 of each afe
 * @details RDAUXB command is sent through SPI, and converted values are read from cell register group B
 *          Data in the register will be the form: {GPIO4, AUXB2, AUXB3, PEC}
 *          Only GPIO4 value is stored, as well as the PEC.
 *          PEC of data is compared to PEC read to check for validity
 *          Values are stored in the `afe->aux_voltages` array, if device_cell for device is enabled
 * @param   afe Pointer to LtcAfeStorage struct, stores runtime data and settings of AFE
 * @param   device_cell The GPIO port we want to read from
 * @return  STATUS_CODE_OK if auxillary input was read correctly
 *          STATUS_CODE_INTERNAL_ERROR if data PEC and PEC read are not the same
 *          STATUS_CODE_INVALID_ARGS if arguments are invalid in `prv_read_register`
 */
StatusCode ltc_afe_read_aux(LtcAfeStorage *afe, uint8_t device_cell);

/**
 * @brief   Mark cell for discharging in each device
 * @details Device number is reverse indexed, since `ltc_afe_write_config` writes configs in reverse order
 *          Appropriate bit in `discharge_bitset` in `LtcAfeConfigRegisterData` is marked (see `ltc_afe_regs.h`)
 *          Only takes effect when config is rewritten with `ltc_afe_write_config`
 * @param   afe Pointer to LtcAfeStorage struct, stores runtime data and settings of AFE
 * @param   cell Index of cell at device x (cell = cell # in device + (# of cells * device_number))
 * @param   discharge Is cell being discharged or not?
 * @return  STATUS_CODE_OK if cell discharge was correctly toggled correctly
 *          STATUS_CODE_INVALID_ARGS if `cell` index is invalid
 */
StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge);

/**
 * @brief   Sets the same discharge PWM duty cycle for all cells across all AFE devices
 * @details Configures the PWM duty cycle registers on each AFE device so that all 12 cell discharge
 *          channels operate at the same specified duty cycle. Each register controls 2 channels, so
 *          6 registers are written per device. Prepares a write command and transmits it to all AFE devices.
 * @param   afe Pointer to LtcAfeStorage struct containing runtime data and settings
 * @param   duty_cycle 4-bit duty cycle value (0x0 to 0xF) to set for all PWM channels
 * @return  STATUS_CODE_OK if the PWM configuration was successfully transmitted
 *          STATUS_CODE_INVALID_ARGS if parameters passed to spi_exchange are invalid
 *          STATUS_CODE_INTERNAL_ERROR if SPI transmission fails
 */
StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle);

/** @} */
