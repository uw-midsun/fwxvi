/************************************************************************************************
 * @file   ltc_afe.c
 *
 * @brief  Source file for the LTC8611 AFE driver
 *
 * @date   2025-04-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <string.h>

/* Macro to calculate the size of an array */
#define SIZEOF_ARRAY(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Inter-component Headers */
#include "delay.h"
#include "log.h"
#include "status.h"

/* Intra-component Headers */
#include "ltc_afe.h"
#include "ltc_afe_crc15.h"
#include "ltc_afe_regs.h"

static bool s_discharge[LTC_AFE_MAX_CELLS];

/* Used as a wrapper internally, since in some functions we don't want the default gpio_pins */
static StatusCode s_write_config(LtcAfeStorage *afe, uint8_t gpio_enable_pins) {
  return STATUS_CODE_OK;
}

/**
 * Calculate the cell result and discharge cell index mappings for the enabled cells across all AFEs.
 *
 * Disabled cells are ignored, so the enabled cells are mapped to a smaller, but continuous,
 * index for storing cell results.
 */
static void s_calc_offsets(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = afe->settings;

  size_t enabled_cell_index = 0;

  for (size_t device = 0; device < settings->num_devices; device++) {
    size_t device_offset = device * LTC_AFE_MAX_CELLS_PER_DEVICE; /* Device-level offset */
    uint16_t bitmask = settings->cell_bitset[device];             /* Extract bitmask once, revealing enabled cells */

    for (size_t device_cell = 0; device_cell < LTC_AFE_MAX_CELLS_PER_DEVICE; device_cell++) {
      size_t raw_cell_index = device_offset + device_cell; /* Absolute index of cell in current device across all devices */

      /* Check if current cell is enabled through the given bitmask */
      if ((bitmask >> device_cell) & 0x1) {
        /* Map enabled cell to result and discharge arrays to separate handling of cell measurement and discharge */
        afe->discharge_cell_lookup[enabled_cell_index] = raw_cell_index;
        afe->cell_result_lookup[raw_cell_index] = enabled_cell_index++;
      }
    }
  }
}

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *config) {
  /* Validate configuration settings based on device limitations */
  if (config->num_devices > LTC_AFE_MAX_DEVICES) {
    LOG_DEBUG("AFE: Configured device count exceeds user-defined limit. Update LTC_AFE_MAX_DEVICES if necessary.");
    return STATUS_CODE_INVALID_ARGS;
  }
  if (config->num_cells > LTC_AFE_MAX_CELLS) {
    LOG_DEBUG("AFE: Configured cell count exceeds device limitations.");
    return STATUS_CODE_INVALID_ARGS;
  }
  if (config->num_thermistors > LTC_AFE_MAX_THERMISTORS) {
    LOG_DEBUG("AFE: Configured thermistor count exceeds limitations.");
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Initialize memory of AFE structure: reset values and copy configuration settings */
  memset(afe, 0, sizeof(LtcAfeSettings));
  memcpy(afe->settings, config, sizeof(LtcAfeSettings));
  
  /* Calculate offset for cell result array due to some cells being disabled */
  s_calc_offsets(storage);

  /* Initialize 15-bit CRC lookup table to optimize packet error code (PEC) calculation */
  crc15_init_table();

  /* Set same duty cycle for all cells in the AFE system */
  StatusCode status = ltc_afe_set_discharge_pwm_cycle(storage, LTC6811_PWMC_DC_100);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Read register failed");
    return status;
  }
  /*
   * Write configuration settings to AFE.
   * GPIO pins are configured with pull-down off: GPIO1 as analog input and GPIO3-5 for SPI
   */
  return ltc_afe_write_config(storage);
}

StatusCode ltc_afe_write_config(LtcAfeStorage *afe) {
  // todo: store config register info?
  return STATUS_CODE_OK;
}

StatusCode ltc_afe_trigger_cell_conv(LtcAfeStorage *afe) {
  return STATUS_CODE_OK;
}

/* Start GPIOs ADC conversion and poll status for XXXXX */
StatusCode ltc_afe_trigger_aux_conv(LtcAfeStorage *afe, uint8_t thermistor) {
  return STATUS_CODE_OK;
}

StatusCode ltc_afe_read_cells(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = afe->settings;

  /* Loop through 4 register groups */
  for (LtcAfeVoltageRegister v_reg_group = LTC_AFE_VOLTAGE_REGISTER_A; v_reg_group < NUM_LTC_AFE_VOLTAGE_REGISTERS; ++v_reg_group) {

    /* Loop through the number of AFE devices connected for each voltage group */
    for (uint8_t device = 0; device < settings->num_devices; ++device) {
 
      /* Loop through 3 voltages per register */
      for (uint16_t cell = 0; cell < LTC6811_CELLS_IN_REG; ++cell) {

        /* Determine index of the battery cell */
        uint16_t device_cell = cell + (v_reg_group * LTC6811_CELLS_IN_REG);
        uint16_t index = device_cell + (device * LTC_AFE_MAX_CELLS_PER_DEVICE);

        /* Read voltage if cell status is enabled */
        if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
          LOG_DEBUG("Cell %d Voltage: %d\n", afe->cell_result_lookup[index], afe->cell_voltages[afe->cell_result_lookup[index]]);
        }
        else {
          LOG_DEBUG("Cell %d disabled\n", afe->cell_result_lookup[index]);
        }
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode ltc_afe_set_voltage(LtcAfeStorage *afe, uint16_t voltage) {
  LtcAfeSettings *settings = afe->settings;

    /* Loop through 4 register groups */
    for (LtcAfeVoltageRegister v_reg_group = LTC_AFE_VOLTAGE_REGISTER_A; v_reg_group < NUM_LTC_AFE_VOLTAGE_REGISTERS; ++v_reg_group) {

      /* Loop through the number of AFE devices connected for each voltage group */
      for (uint8_t device = 0; device < settings->num_devices; ++device) {
  
        /* Loop through 3 voltages per register */
        for (uint16_t cell = 0; cell < LTC6811_CELLS_IN_REG; ++cell) {

          /* Determine index of the battery cell */
          uint16_t device_cell = cell + (v_reg_group * LTC6811_CELLS_IN_REG);
          uint16_t index = device_cell + (device * LTC_AFE_MAX_CELLS_PER_DEVICE);

          /* Set voltages for enabled cells */
          if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
            afe->cell_voltages[afe->cell_result_lookup[index]] = voltage;
          }
        }
      }
    }
    return STATUS_CODE_OK;
}

StatusCode ltc_afe_set_aux(LtcAfeStorage *afe, uint8_t device_cell, uint16_t voltage) {
  LtcAfeSettings *settings = afe->settings;

  size_t len = settings->num_devices * sizeof(LtcAfeAuxData);

  /* Loop through devices */
  for (uint16_t device = 0; device < settings->num_devices; ++device) {
    /* If pin not available - move on */
    if (!((settings->aux_bitset[device] >> device_cell) & 0x1)) {
      LOG_DEBUG(
          "Device: %u\n"
          "Device Cell: %u\n"
          "Not available.\n",
          device, device_cell);
      continue;
    }

    uint16_t index = device * LTC_AFE_MAX_THERMISTORS_PER_DEVICE + device_cell;
    afe->aux_voltages[index] = voltage;
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_read_aux(LtcAfeStorage *afe, uint8_t device_cell) {
  LtcAfeSettings *settings = afe->settings;

  size_t len = settings->num_devices * sizeof(LtcAfeAuxData);

  /* Loop through devices */
  for (uint16_t device = 0; device < settings->num_devices; ++device) {
    /* If pin not available - move on */
    if (!((settings->aux_bitset[device] >> device_cell) & 0x1)) {
      LOG_DEBUG(
          "Device: %u\n"
          "Device Cell: %u\n"
          "Not available.\n",
          device, device_cell);
      continue;
    }

    uint16_t index = device * LTC_AFE_MAX_THERMISTORS_PER_DEVICE + device_cell;
    LOG_DEBUG("Aux voltage: %d\n", afe->aux_voltages[index]);
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  if (cell > LTC_AFE_MAX_CELLS) {
    LOG_DEBUG("Cell number is out of bounds");
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t cell_in_lookup = afe->discharge_cell_lookup[cell];
  uint16_t device_index = cell_in_lookup / LTC_AFE_MAX_CELLS_PER_DEVICE;
  uint16_t device = afe->settings->num_devices - device_index - 1; /* devices are reverse indexed cz of daisy chain config */

  uint16_t device_cell_bit = cell_in_lookup % LTC_AFE_MAX_CELLS_PER_DEVICE;

  LtcAfeConfigRegisterData *config_reg_data = &afe->device_configs->devices[device].reg;

  if (discharge) {
    config_reg_data->discharge_bitset |= (1 << (device_cell_bit));
  } else {
    config_reg_data->discharge_bitset &= ~(1 << (device_cell_bit));
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle) {

  /* Duty cycle must be between 0x0 - 0xF */
  if (duty_cycle > 0xF) return STATUS_CODE_INVALID_ARGS;
  s_duty_cycle = duty_cycle;
  return STATUS_CODE_OK;
}
