/************************************************************************************************
 * @file   adbms_afe_x86.c
 *
 * @brief  Source file for the ADBMS8611 AFE x86 driver
 *
 * @date   2025-06-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* DELETE*/

/* Standard library Headers */
#include <stddef.h>
#include <string.h>

/* Inter-component Headers */
#include "delay.h"
#include "log.h"
#include "status.h"

/* Intra-component Headers */
#include "adbms_afe.h"
#include "adbms_afe_crc15.h"
#include "adbms_afe_regs.h"

static AdbmsAfeStorage *p_afe = NULL;

/**
 * Calculate the cell result and discharge cell index mappings for the enabled cells across all AFEs.
 *
 * Disabled cells are ignored, so the enabled cells are mapped to a smaller, but continuous,
 * index for storing cell results.
 */

static void s_calc_offsets(AdbmsAfeStorage *afe) {
  if (afe == NULL) {
    return;
  }

  AdbmsAfeSettings *settings = afe->settings;

  size_t enabled_cell_index = 0;

  for (size_t device = 0; device < settings->num_devices; device++) {
    size_t device_offset = device * ADBMS_AFE_MAX_CELLS_PER_DEVICE; /* Device-level offset */
    uint16_t bitmask = settings->cell_bitset[device];               /* Extract bitmask once, revealing enabled cells */

    for (size_t device_cell = 0; device_cell < ADBMS_AFE_MAX_CELLS_PER_DEVICE; device_cell++) {
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

AdbmsAfeStorage *adbms_afe_get_storage(void) {
  return p_afe;
}

StatusCode adbms_afe_init(AdbmsAfeStorage *afe, const AdbmsAfeSettings *config) {
  if (config->num_devices > ADBMS_AFE_MAX_DEVICES) {
    LOG_DEBUG("AFE: Configured device count exceeds user-defined limit. Update ADBMS_AFE_MAX_DEVICES if necessary.");
    return STATUS_CODE_INVALID_ARGS;
  }
  if (config->num_cells > ADBMS_AFE_MAX_CELLS) {
    LOG_DEBUG("AFE: Configured cell count exceeds device limitations.");
    return STATUS_CODE_INVALID_ARGS;
  }
  if (config->num_thermistors > ADBMS_AFE_MAX_CELL_THERMISTORS) {
    LOG_DEBUG("AFE: Configured thermistor count exceeds limitations.");
    return STATUS_CODE_INVALID_ARGS;
  }

  p_afe = afe;
  memset(afe, 0, sizeof(*afe));

  afe->settings = config;

  /* Calculate offset for cell result array due to some cells being disabled */
  s_calc_offsets(afe);

  /* Initialize 15-bit CRC lookup table to optimize packet error code (PEC) calculation */
  crc15_init_table();

  return adbms_afe_write_config(afe);
}

StatusCode adbms_afe_write_config(AdbmsAfeStorage *afe) {
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_trigger_cell_conv(AdbmsAfeStorage *afe) {
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_trigger_thermistor_conv(AdbmsAfeStorage *afe, uint8_t device_num, uint8_t index) {
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_trigger_board_temp_conv(AdbmsAfeStorage *afe, uint8_t device_num) {
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_read_cells(AdbmsAfeStorage *afe) {
  if (afe == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  AdbmsAfeSettings *settings = afe->settings;

  /* Loop through 4 register groups */
  for (AdbmsAfeVoltageRegister v_reg_group = ADBMS_AFE_VOLTAGE_REGISTER_A; v_reg_group < NUM_ADBMS_AFE_VOLTAGE_REGISTERS; ++v_reg_group) {
    /* Loop through the number of AFE devices connected for each voltage group */
    for (uint8_t device = 0; device < settings->num_devices; ++device) {
      /* Loop through 3 voltages per register */
      for (uint16_t cell = 0; cell < ADBMS1818_CELLS_IN_REG; ++cell) {
        /* Determine index of the battery cell */
        uint16_t device_cell = cell + (v_reg_group * ADBMS1818_CELLS_IN_REG);
        uint16_t index = device_cell + (device * ADBMS_AFE_MAX_CELLS_PER_DEVICE);

        /* Read voltage if cell status is enabled */
        if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
          LOG_DEBUG("Cell %d Voltage: %d\n", afe->cell_result_lookup[index], afe->cell_voltages[afe->cell_result_lookup[index]]);
        } else {
          LOG_DEBUG("Cell %d disabled\n", afe->cell_result_lookup[index]);
        }
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_read_thermistor(AdbmsAfeStorage *afe, uint8_t device_num, uint8_t thermistor_index) {
  if (afe == NULL || device_num >= ADBMS_AFE_MAX_DEVICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t index = device_num * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor_index;
  LOG_DEBUG("Board %d Thermistor %d voltage: %d\n", device_num, thermistor_index, afe->board_thermistor_voltages[index]);
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_read_board_temp(AdbmsAfeStorage *afe, uint8_t device_num) {
  if (afe == NULL || device_num >= ADBMS_AFE_MAX_DEVICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  LOG_DEBUG("Board %d thermistor voltage: %d\n", device_num, afe->board_thermistor_voltages[device_num]);
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_toggle_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell, bool discharge) {
  if (!afe || !afe->settings) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  const uint16_t num_devices = afe->settings->num_devices;
  const uint16_t cells_per_dev = ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  const uint32_t total_cells = (uint32_t)num_devices * cells_per_dev;

  if (cell >= total_cells) {
    LOG_DEBUG("Cell %u out of bounds (0..%u)", cell, total_cells - 1);
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t phys_cell = cell;

  uint16_t dev_idx = phys_cell / cells_per_dev;
  uint16_t device = num_devices - dev_idx - 1;

  uint16_t cell_indx_in_dev = phys_cell % cells_per_dev;

  if (cell_indx_in_dev < 12) {
    /* Cells 0-11 belong to CFGRA */
    AdbmsAfeConfigRegisterAData *cfgA = &afe->config_a[dev_idx].cfg;

    if (discharge) {
      cfgA->discharge_bitset |= (1U << cell_indx_in_dev);
    } else {
      cfgA->discharge_bitset &= ~(1U << cell_indx_in_dev);
    }

  } else {
    /* Cells 12-17 belong to CFGRB */
    AdbmsAfeConfigRegisterBData *cfgB = &afe->config_b[dev_idx].cfg;

    uint8_t bit_index = cell_indx_in_dev - 12;
    if (discharge) {
      cfgB->discharge_bitset |= (1U << bit_index);
    } else {
      cfgB->discharge_bitset &= ~(1U << bit_index);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_discharge_pwm_cycle(AdbmsAfeStorage *afe, uint8_t duty_cycle) {
  LOG_DEBUG("Discharge PWM duty cycle set to %u", duty_cycle);
  return STATUS_CODE_OK;
}

/* SETTERS AND GETTERS */

StatusCode adbms_afe_set_cell_voltage(AdbmsAfeStorage *afe, uint8_t cell_index, float voltage) {
  if (afe == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  size_t cell_count = afe->settings->num_cells;
  if (cell_index >= cell_count) {
    LOG_DEBUG("Invalid cell index number (%u), cell count is %zu", cell_index, cell_count);

    return STATUS_CODE_INVALID_ARGS;
  }

  size_t device_index = cell_index / ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  size_t cell_pin = cell_index % ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  // if (!((afe->settings->cell_bitset[device_index] >> cell_pin) & 0x1)) {
  //   LOG_DEBUG("Channel %zu on device %zu is disabled; skipping set.\n", cell_pin, device_index);
  //   return STATUS_CODE_OK;
  // }

  afe->cell_voltages[cell_index] = voltage;
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_thermistor_voltage(AdbmsAfeStorage *afe, uint8_t device_num, uint8_t thermistor_index, float voltage) {
  if (afe == NULL || thermistor_index >= ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE || device_num >= ADBMS_AFE_MAX_DEVICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t index = device_num * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor_index;
  afe->thermistor_voltages[index] = voltage;

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_board_thermistor_voltage(AdbmsAfeStorage *afe, uint8_t device_num, float voltage) {
  if (afe == NULL || device_num >= ADBMS_AFE_MAX_DEVICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  afe->board_thermistor_voltages[device_num] = voltage;
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_afe_dev_cell_voltages(AdbmsAfeStorage *afe, size_t device_num, float voltage) {
  if (afe == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  size_t afe_count = afe->settings->num_devices;

  if (device_num >= afe_count) {
    LOG_DEBUG("Afe device does not exist, invalid index");
    return STATUS_CODE_INVALID_ARGS;
  }

  const uint8_t start = device_num * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  const uint8_t end = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  for (uint8_t cell_index = start; cell_index < end; ++cell_index) {
    adbms_afe_set_cell_voltage(afe, cell_index, voltage);
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_afe_dev_thermistor_voltages(AdbmsAfeStorage *afe, size_t device_num, float voltage) {
  if (afe == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  size_t afe_count = afe->settings->num_devices;

  if (device_num >= afe_count) {
    LOG_DEBUG("Afe device does not exist, invalid index");
    return STATUS_CODE_INVALID_ARGS;
  }

  for (uint8_t thermistor_index = 0; thermistor_index < ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE; ++thermistor_index) {
    adbms_afe_set_thermistor_voltage(afe, device_num, thermistor_index, voltage);
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_pack_cell_voltages(AdbmsAfeStorage *afe, float voltage) {
  if (afe == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t device_count = afe->settings->num_devices;
  for (uint8_t device = 0; device < device_count; ++device) {
    adbms_afe_set_afe_dev_cell_voltages(afe, device, voltage);
  }
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_pack_thermistor_voltages(AdbmsAfeStorage *afe, float voltage) {
  if (afe == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t device_count = afe->settings->num_devices;
  for (uint8_t device = 0; device < device_count; ++device) {
    adbms_afe_set_afe_dev_thermistor_voltages(afe, device, voltage);
  }
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_pack_board_thermistor_voltages(AdbmsAfeStorage *afe, float voltage) {
  if (afe == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t device_count = afe->settings->num_devices;
  for (uint8_t device = 0; device < device_count; ++device) {
    adbms_afe_set_board_thermistor_voltage(afe, device, voltage);
  }
  return STATUS_CODE_OK;
}

uint16_t adbms_afe_get_cell_voltage(AdbmsAfeStorage *afe, uint16_t cell_index) {
  if (afe == NULL) {
    return 0;
  }

  return afe->cell_voltages[cell_index];
}

uint16_t adbms_afe_get_thermistor_voltage(AdbmsAfeStorage *afe, uint8_t device_num, uint16_t thermistor_index) {
  if (afe == NULL) {
    return 0;
  }

  uint16_t index = device_num * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor_index;
  return afe->thermistor_voltages[index];
}

uint16_t adbms_afe_get_board_thermistor_voltage(AdbmsAfeStorage *afe, uint8_t device_num) {
  if (afe == NULL) {
    return 0;
  }

  return afe->board_thermistor_voltages[device_num];
}

bool adbms_afe_get_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell) {
  if (afe == NULL) {
    return 0;
  }

  bool discharge = 0;
  uint16_t cell_in_lookup = cell;
  uint16_t device_index = cell_in_lookup / ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  uint16_t device = afe->settings->num_devices - device_index - 1;
  uint16_t bit_position = cell_in_lookup % ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  if (bit_position < 12) {
    uint16_t bitset = afe->config_a[device].cfg.discharge_bitset;
    discharge = (bitset >> bit_position) & 0x1;
  } else {
    uint16_t b_bit = bit_position - 12;
    uint16_t bitset = afe->config_b[device].cfg.discharge_bitset;
    discharge = (bitset >> b_bit) & 0x1;
  }

  return discharge;
}
