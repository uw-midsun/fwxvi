#pragma once

/************************************************************************************************
 * @file   adbms_afe.h
 *
 * @brief  Header file for the ADBMS1818 AFE driver
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
#include "adbms_afe_regs.h"
#include "status.h"

/**
 * @defgroup ADBMS1818
 * @brief    ADBMS1818 library
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
#define ADBMS_AFE_MAX_DEVICES 3

/**
 * @brief Maximum x of each device
 * @note This is a device limitation
 */
#define ADBMS_AFE_MAX_CELLS_PER_DEVICE 18
#define ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE 9

/**
 * @brief Maximum across all devices
 */
#define ADBMS_AFE_MAX_CELLS (ADBMS_AFE_MAX_DEVICES * ADBMS_AFE_MAX_CELLS_PER_DEVICE)
#define ADBMS_AFE_MAX_THERMISTORS (ADBMS_AFE_MAX_DEVICES * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE)

/**
 * @brief   Select the ADC mode
 * @details Trade-off between speed or minimizing noise
 * @note    See p 50 for conversion times and p 23 (table 3) for noise
 */
typedef enum {
  ADBMS_AFE_ADC_MODE_422HZ = 0,
  ADBMS_AFE_ADC_MODE_27KHZ, /**< Normal */
  ADBMS_AFE_ADC_MODE_7KHZ,  /**< Fast */
  ADBMS_AFE_ADC_MODE_26HZ,  /**< Filtered */
  ADBMS_AFE_ADC_MODE_1KHZ,
  ADBMS_AFE_ADC_MODE_14KHZ,
  ADBMS_AFE_ADC_MODE_3KHZ,
  ADBMS_AFE_ADC_MODE_2KHZ,
  NUM_ADBMS_AFE_ADC_MODES
} AdbmsAfeAdcMode;

/**
 * @brief   Afe Settings Data
 * @details Set by the user when `adbms_afe_init` is called
 *          Stores SPI information, which cell and aux inputs are enabled, and number of things
 * @note    For more info on `SpiSettings` refer to `spi.h`
 */
typedef struct AdbmsAfeSettings {
  AdbmsAfeAdcMode adc_mode; /**< Determines ADC Mode */

  uint16_t cell_bitset[ADBMS_AFE_MAX_DEVICES]; /**< Bitset showing cells are enabled for each device */
  uint16_t aux_bitset[ADBMS_AFE_MAX_DEVICES];  /**< Bitset showing aux inputs enabled for each device */

  size_t num_devices;     /**< Number of AFE devices */
  size_t num_cells;       /**< Number of TOTAL cells across all devices */
  size_t num_thermistors; /**< Number of TOTAL thermistors (aux inputs) across all devices */

  SpiSettings spi_settings; /**< SPI settings for AFE */
  const SpiPort spi_port;   /**< Determines which SPI port to use */
} AdbmsAfeSettings;

/**
 * @brief   Runtime Data Storage
 * @details Stores settings, configs, and voltages for all AFE devices
 * @note    Raw indices mean
 *          - Index `0` corresponds to the first cell/aux value of the first AFE device.
 *          - Index `n * max_x_per_device` corresponds to the first value of the (n+1)th AFE device.
 *          - Each AFE's data (voltages, lookups, etc.) is stored contiguously before the next AFE's data.
 */
typedef struct AdbmsAfeStorage {
  uint16_t cell_voltages[ADBMS_AFE_MAX_CELLS];      /**< Stores cell voltages for all devices */
  uint16_t aux_voltages[ADBMS_AFE_MAX_THERMISTORS]; /**< Stores aux voltages for all devices */

  uint16_t cell_result_lookup[ADBMS_AFE_MAX_CELLS];      /**< Map raw cell indices read from AFE to `cell_voltages` */
  uint16_t aux_result_lookup[ADBMS_AFE_MAX_THERMISTORS]; /**< Map raw aux input indices read from AFE to `aux_voltages` */
  uint16_t discharge_cell_lookup[ADBMS_AFE_MAX_CELLS];   /**< Map indicies of `cell_voltages` to raw cell indices */

  AdbmsAfeSettings *settings;                /**< Stores settings for AFE devices, set by the user */
  AdbmsAfeWriteConfigPacket *device_configs; /**< Stores the Configuration of each device in the CFGR register */
} AdbmsAfeStorage;

/**
 * @brief   Initializes the ADBMS AFE system with provided configuration settings
 * @details Validates user-defined configuration parameters, sets up SPI communication,
 *          calculates result buffer offsets, initializes the CRC15 lookup table,
 *          and writes configuration settings to all AFE devices
 * @param   afe Pointer to AdbmsAfeStorage struct, stores runtime data and settings of AFE
 * @param   config Pointer to constant AdbmsAfeSettings struct, contains user-specified configuration parameters
 * @return  STATUS_CODE_OK if initialization is successful
 *          STATUS_CODE_INVALID_ARGS if device, cell, or thermistor counts exceed defined maximums
 *          STATUS_CODE_INTERNAL_ERROR if SPI or configuration writes fail during setup
 */
StatusCode adbms_afe_init(AdbmsAfeStorage *afe, const AdbmsAfeSettings *config);

/**
 * @brief   Writes configuration bits onto CFGR (Configuration Register Group)
 * @details Writes configuration bits to each of the AFE devices' configuration register bits
 * @param   afe Pointer to AdbmsAfeStorage struct, stores runtime data and settings of AFE
 * @return  STATUS_CODE_OK if configs were written onto register
 *          STATUS_CODE_INVALID_ARGS if one of the parameters in `spi_exchange` is incorrect in `prv_write_config`
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission/receiving fails
 *          STATUS_CODE_TIMEOUT if transmission/receiving takes too long
 * @note    Only for initial config
 */
StatusCode adbms_afe_write_config(AdbmsAfeStorage *afe);

/**
 * @brief   Triggers ADC conversion for all enabled cell voltage inputs
 * @details Builds and transmits the ADCV command using the configured ADC mode to start
 *          cell voltage conversion across all AFE devices. Puts device into active mode
 *          before initiating conversion.
 * @param   afe Pointer to AdbmsAfeStorage struct containing runtime data and settings
 * @return  STATUS_CODE_OK if the ADCV command was successfully transmitted
 *          STATUS_CODE_INVALID_ARGS if arguments passed to spi_exchange are invalid
 *          STATUS_CODE_INTERNAL_ERROR if SPI transmission fails
 */
StatusCode adbms_afe_trigger_cell_conv(AdbmsAfeStorage *afe);

/**
 * @brief   Triggers ADC conversion for an auxiliary (thermistor) input
 * @details Selects the specified thermistor by configuring GPIO bits, then builds and transmits
 *          the ADAX command to begin auxiliary ADC conversion using GPIO4. Supports thermistors
 *          mapped to 5 available GPIO inputs across devices.
 * @param   afe Pointer to AdbmsAfeStorage struct containing runtime data and settings
 * @param   thermistor Index of the thermistor to measure (0-7 across devices)
 * @return  STATUS_CODE_OK if the ADAX command was successfully transmitted
 *          STATUS_CODE_INVALID_ARGS if arguments passed to spi_exchange are invalid
 *          STATUS_CODE_INTERNAL_ERROR if SPI transmission fails
 */
StatusCode adbms_afe_trigger_aux_conv(AdbmsAfeStorage *afe, uint8_t device_cell);

/**
 * @brief   Reads and stores cell voltages from the voltage cell registers of each afe
 * @details RDCVx (x = A, B, C, D) command is sent to read converted voltage from respective register group
 *          Received PEC and PEC of data is compared to see if data is valid
 *          Voltages are stored in the `afe->cell_voltages` array, if cell status is enabled
 * @param   afe Pointer to AdbmsAfeStorage struct, stores runtime data and settings of AFE
 * @return  STATUS_CODE_OK if cells were read correctly
 *          STATUS_CODE_INTERNAL_ERROR if read PEC and calcualted PEC do not match up
 *          STATUS_CODE_INVALID_ARGS if arguments are invalid in `prv_read_register`
 */
StatusCode adbms_afe_read_cells(AdbmsAfeStorage *afe);

/**
 * @brief   Reads and stores auxillary input from GPIO4 of each afe
 * @details RDAUXB command is sent through SPI, and converted values are read from cell register group B
 *          Only GPIO4 and GPIO5 value is stored, as well as the PEC.
 *          PEC of data is compared to PEC read to check for validity
 *          Values are stored in the `afe->aux_voltages` array, if device_cell for device is enabled
 * @note    See Table 63 (p.66) for more details
 * @param   afe Pointer to AdbmsAfeStorage struct, stores runtime data and settings of AFE
 * @param   device_cell The GPIO port we want to read from
 * @return  STATUS_CODE_OK if auxillary input was read correctly
 *          STATUS_CODE_INTERNAL_ERROR if data PEC and PEC read are not the same
 *          STATUS_CODE_INVALID_ARGS if arguments are invalid in `prv_read_register`
 */
StatusCode adbms_afe_read_aux(AdbmsAfeStorage *afe, uint8_t device_cell);

/**
 * @brief   Mark cell for discharging in each device
 * @details Device number is reverse indexed, since `adbms_afe_write_config` writes configs in reverse order
 *          Appropriate bit in `discharge_bitset` in `AdbmsAfeConfigRegisterData` is marked (see `adbms_afe_regs.h`)
 *          Only takes effect when config is rewritten with `adbms_afe_write_config`
 * @param   afe Pointer to AdbmsAfeStorage struct, stores runtime data and settings of AFE
 * @param   cell Index of cell at device x (cell = cell # in device + (# of cells * device_number))
 * @param   discharge Is cell being discharged or not?
 * @return  STATUS_CODE_OK if cell discharge was correctly toggled correctly
 *          STATUS_CODE_INVALID_ARGS if `cell` index is invalid
 */
StatusCode adbms_afe_toggle_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell, bool discharge);

/**
 * @brief   Sets the same discharge PWM duty cycle for all cells across all AFE devices
 * @details Configures the PWM duty cycle registers on each AFE device so that all 18 cell discharge
 *          channels operate at the same specified duty cycle. Each register controls 2 channels, so
 *          6 registers are written per device. Prepares a write command and transmits it to all AFE devices.
 * @param   afe Pointer to AdbmsAfeStorage struct containing runtime data and settings
 * @param   duty_cycle 4-bit duty cycle value (0x0 to 0xF) to set for all PWM channels
 * @return  STATUS_CODE_OK if the PWM configuration was successfully transmitted
 *          STATUS_CODE_INVALID_ARGS if parameters passed to spi_exchange are invalid
 *          STATUS_CODE_INTERNAL_ERROR if SPI transmission fails
 */
StatusCode adbms_afe_set_discharge_pwm_cycle(AdbmsAfeStorage *afe, uint8_t duty_cycle);

#ifdef MS_PLATFORM_X86
#ifdef __cplusplus 
extern "C" {
#endif
extern AdbmsAfeStorage s_afe;
extern AdbmsAfeSettings s_settings;
#ifdef __cplusplus
}
#endif
/**
 * @brief Sets the voltage for a specific cell in the AFE.
 * @param afe Pointer to the AFE storage structure.
 * @param cell_index Index of the cell (global index across all devices).
 * @param voltage The voltage value to set.
 * @return STATUS_CODE_OK if successful, STATUS_CODE_INVALID_ARGS if the index is out of range.
 */
StatusCode adbms_afe_set_cell_voltage(AdbmsAfeStorage *afe, uint8_t cell_index, float voltage);

/**
 * @brief Sets the voltage for a specific auxiliary input (thermistor channel).
 * @param afe Pointer to the AFE storage structure.
 * @param aux_index Index of the auxiliary channel (global index across all devices).
 * @param voltage The voltage value to set.
 * @return STATUS_CODE_OK if successful, STATUS_CODE_INVALID_ARGS if the index is out of range.
 */
StatusCode adbms_afe_set_aux_voltage(AdbmsAfeStorage *afe, uint8_t aux_index, float voltage);

/**
 * @brief Sets the voltage for all cells on a specific AFE device.
 * @param afe Pointer to the AFE storage structure.
 * @param afe_index Index of the AFE device.
 * @param voltage The voltage value to set for each cell.
 * @return STATUS_CODE_OK if successful, STATUS_CODE_INVALID_ARGS if the device index is invalid.
 */
StatusCode adbms_afe_set_afe_dev_cell_voltages(AdbmsAfeStorage *afe, size_t afe_index, float voltage);

/**
 * @brief Sets the voltage for all auxiliary inputs on a specific AFE device.
 * @param afe Pointer to the AFE storage structure.
 * @param afe_index Index of the AFE device.
 * @param voltage The voltage value to set for each auxiliary channel.
 * @return STATUS_CODE_OK if successful, STATUS_CODE_INVALID_ARGS if the device index is invalid.
 */
StatusCode adbms_afe_set_afe_dev_aux_voltages(AdbmsAfeStorage *afe, size_t afe_index, float voltage);

/**
 * @brief Sets the voltage for all cells across all AFE devices in the pack.
 * @param afe Pointer to the AFE storage structure.
 * @param voltage The voltage value to set for all cells.
 * @return STATUS_CODE_OK always.
 */
StatusCode adbms_afe_set_pack_cell_voltages(AdbmsAfeStorage *afe, float voltage);

/**
 * @brief Sets the voltage for all auxiliary channels across all AFE devices in the pack.
 * @param afe Pointer to the AFE storage structure.
 * @param voltage The voltage value to set for all auxiliary inputs.
 * @return STATUS_CODE_OK always.
 */
StatusCode adbms_afe_set_pack_aux_voltages(AdbmsAfeStorage *afe, float voltage);

/**
 * @brief Gets the simulated voltage for a specific cell.
 * @param index Global index of the cell.
 * @return The voltage value of the specified cell.
 */
uint16_t adbms_afe_get_cell_voltage(AdbmsAfeStorage *afe, uint16_t index);

/**
 * @brief Gets the simulated voltage for a specific auxiliary input.
 * @param index Global index of the auxiliary input.
 * @return The voltage value of the specified auxiliary input.
 */
uint16_t adbms_afe_get_aux_voltage(AdbmsAfeStorage *afe, uint16_t index);

/**
 * @brief   Get the discharge enable state of a single cell.
 * @param   afe Pointer to AFE storage/configuration.
 * @param   cell Global cell index (0…ADBMS_AFE_MAX_CELLS‑1).
 * @return  Bool returning if cell discharge is enabled or disabled
 */
bool adbms_afe_get_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell);
#endif
/** @} */
