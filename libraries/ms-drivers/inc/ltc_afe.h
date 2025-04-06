#pragma once
// Driver for LTC6811 AFE chip

// TODO(SOFT-9): Need to update GPIO/ADC part

// Requires GPIO, Interrupts, Soft Timers, and Event Queue to be initialized

// Note that all units are in 100uV.

// This module supports AFEs with >=12 cells using the |cell/aux_bitset|.
// Note that due to the long conversion delays required, we use an FSM to return control to the
// application.
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "fsm.h"
#include "gpio.h"
#include "spi.h"
#include "status.h"

// This is an arbitrary limitation, can be increased/decreased if needed
#define LTC_AFE_MAX_DEVICES 3
// This is a device limitation
#define LTC_AFE_MAX_CELLS_PER_DEVICE 12
#define LTC_AFE_MAX_THERMISTORS_PER_DEVICE 8
#define LTC_AFE_MAX_CELLS (LTC_AFE_MAX_DEVICES * LTC_AFE_MAX_CELLS_PER_DEVICE)
#define LTC_AFE_MAX_THERMISTORS (LTC_AFE_MAX_DEVICES * LTC_AFE_MAX_THERMISTORS_PER_DEVICE)

// Remove padding, if possible
#if defined(__GNUC__)
#define _PACKED __attribute__((packed))
#else
#define _PACKED
#endif

/** 
 * @brief   Select the ADC mode
 * @details Trade-off between speed or minimizing noise
 * @note    See p 50 for conversion times and p 23 (table 3) for noise 
 */
typedef enum { 
  LTC_AFE_ADC_MODE_27KHZ = 0, 
  LTC_AFE_ADC_MODE_7KHZ, 
  LTC_AFE_ADC_MODE_26HZ, 
  LTC_AFE_ADC_MODE_14KHZ, 
  LTC_AFE_ADC_MODE_3KHZ, 
  LTC_AFE_ADC_MODE_2KHZ, 
  NUM_LTC_AFE_ADC_MODES 
} LtcAfeAdcMode;

/**
 * @brief   Afe Settings Data 
 * @details Set by the user when `ltc_afe_init` is called
 *          Stores SPI information, which cell and aux inputs are enabled, and number of things
 */
typedef struct LtcAfeSettings {
  GpioAddress cs;
  GpioAddress sdo;
  GpioAddress sdi;
  GpioAddress sclk;

  const SpiPort spi_port;
  uint32_t spi_baudrate;

  LtcAfeAdcMode adc_mode;                    /**< Determines ADC Mode */

  uint16_t cell_bitset[LTC_AFE_MAX_DEVICES]; /**< Bitset showing cells are enabled for each device */
  uint16_t aux_bitset[LTC_AFE_MAX_DEVICES];  /**< Bitset showing aux inputs enabled for each device */

  size_t num_devices;                        /**< Number of AFE devices */
  size_t num_cells;                          /**< Number of TOTAL cells across all devices */
  size_t num_thermistors;                    /**< Number of TOTAL thermistors (aux inputs) across all devices */

  void *result_context;
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
  uint16_t cell_voltages[LTC_AFE_MAX_CELLS];            /**< Stores cell voltages for all devices */
  uint16_t aux_voltages[LTC_AFE_MAX_THERMISTORS];       /**< Stores aux voltages for all devices */

  uint16_t cell_result_lookup[LTC_AFE_MAX_CELLS];       /**< Map raw cell indices read from AFE to `cell_voltages` */
  uint16_t aux_result_lookup[LTC_AFE_MAX_THERMISTORS];  /**< Map raw aux input indices read from AFE to `aux_voltages` */
  uint16_t discharge_cell_lookup[LTC_AFE_MAX_CELLS];    /**< Map indicies of `cell_voltages` to raw cell indices */

  LtcAfeSettings settings;                              /**< Stores settings for AFE devices, set by the user */
  LtcAfeWriteConfigPacket device_configs;               /**< Stores the Configuration of each device in the CFGR register */
  
} LtcAfeStorage;

// Helper functions for the LTC6811
//
// This module is mostly exposed for the FSM. Do not use functions in this module directly.
// Requires SPI, soft timers to be initialized
//
// Assumes that:
// Requires GPIO, Interrupts and Soft Timers to be initialized
//
// Note that all units are in 100uV.
//
// This module supports AFEs with fewer than 12 cells using the |input_bitset|.

// Initialize the LTC6811.
// |settings.cell_bitset| and |settings.aux_bitset| should be an array of bitsets where bits 0 to 11
// represent whether we should monitor the cell input for the given device.
// |settings.cell_result_cb| and |settings.aux_result_cb| will be called when the corresponding
// conversion is completed.
StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *config);

/**
 * @brief   Writes configuration bits onto CFGR (Configuration Register Group)
 * @details Writes configuration bits to each of the AFE devices' configuration register bits
 * @param   afe Pointer to LtcAfeStorage struct, stores runtime data and settings of AFE
 * @return  STATUS_CODE_OK if configs were written onto register
 *          STATUS_CODE_INVALID_ARGS if one of the parameters in `spi_exchange` is incorrect in `prv_write_config`
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission/receiving fails
 *          STATUS_CODE_TIMEOUT if transmission/receiving takes too long
 */
StatusCode ltc_afe_write_config(LtcAfeStorage *afe);

// Triggers a conversion. Note that we need to wait for the conversions to complete before the
// readback will be valid.
StatusCode ltc_afe_trigger_cell_conv(LtcAfeStorage *afe);
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

// Sets the duty cycle to the same value for all cells on all afes
StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle);
