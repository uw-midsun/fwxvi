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
  if (config->num_thermistors > ADBMS_AFE_MAX_THERMISTORS) {
    LOG_DEBUG("AFE: Configured thermistor count exceeds limitations.");
    return STATUS_CODE_INVALID_ARGS;
  }

  p_afe = afe;
  memset(afe, 0, sizeof(*afe));

  /* Initialize memory of AFE structure: reset values and copy configuration settings */
  afe->settings = malloc(sizeof(*afe->settings));
  if (!afe->settings) {
    LOG_DEBUG("Settings Not allocated properly");
    return STATUS_CODE_INTERNAL_ERROR;
  }

  afe->device_configs = calloc(0, sizeof(*afe->device_configs));
  if (!afe->device_configs) {
    LOG_DEBUG("Device Configs not allocated properly");
    return STATUS_CODE_INTERNAL_ERROR;
  }

  memcpy(afe->settings, config, sizeof(AdbmsAfeSettings));

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

StatusCode adbms_afe_trigger_aux_conv(AdbmsAfeStorage *afe, uint8_t device_cell) {
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_read_cells(AdbmsAfeStorage *afe) {
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

StatusCode adbms_afe_read_aux(AdbmsAfeStorage *afe, uint8_t device_cell) {
  AdbmsAfeSettings *settings = afe->settings;

  size_t len = settings->num_devices * sizeof(AdbmsAfeAuxData);

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

    uint16_t index = device * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE + device_cell;
    LOG_DEBUG("Aux voltage: %d\n", afe->aux_voltages[index]);
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_toggle_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell, bool discharge) {
  if (!afe || !afe->settings || !afe->device_configs) {
    LOG_DEBUG("ADBMS AFE not initialized");
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

  uint16_t bit_in_dev = phys_cell % cells_per_dev;

  if (bit_in_dev < 12) {
    AdbmsAfeConfigRegisterAData *cfgA = &afe->device_configs->devices[device].cfgA;

    if (discharge)
      cfgA->discharge_bitset |= (1u << bit_in_dev);
    else
      cfgA->discharge_bitset &= ~(1u << bit_in_dev);
  } else {
    AdbmsAfeConfigRegisterBData *cfgB = &afe->device_configs->devices[device].cfgB;
    uint16_t b_bit = bit_in_dev - 12;
    if (discharge)
      cfgB->discharge_bitset |= (1u << b_bit);
    else
      cfgB->discharge_bitset &= ~(1u << b_bit);
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_discharge_pwm_cycle(AdbmsAfeStorage *afe, uint8_t duty_cycle) {
  LOG_DEBUG("Discharge PWM duty cycle set to %u", duty_cycle);
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_cell_voltage(AdbmsAfeStorage *afe, uint8_t cell_index, float voltage) {
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
StatusCode adbms_afe_set_aux_voltage(AdbmsAfeStorage *afe, uint8_t aux_index, float voltage) {
  size_t thermistors = afe->settings->num_thermistors;
  if (aux_index >= thermistors) {
    LOG_DEBUG("Invalid aux index number");
    return STATUS_CODE_INVALID_ARGS;
  }
  size_t device_index = aux_index / ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;
  size_t aux_pin = aux_index % ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;

  if (!((afe->settings->aux_bitset[device_index] >> aux_pin) & 0x1)) {
    LOG_DEBUG("Aux channel %zu on device %zu is disabled; skipping set.", aux_pin, device_index);
    return STATUS_CODE_OK;
  }

  afe->aux_voltages[aux_index] = voltage;
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_afe_dev_cell_voltages(AdbmsAfeStorage *afe, size_t afe_index, float voltage) {
  size_t afe_count = afe->settings->num_devices;

  if (afe_index >= afe_count) {
    LOG_DEBUG("Afe device does not exist, invalid index");
    return STATUS_CODE_INVALID_ARGS;
  }

  const uint8_t start = afe_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  const uint8_t end = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  for (uint8_t cell_index = start; cell_index < end; ++cell_index) {
    adbms_afe_set_cell_voltage(afe, cell_index, voltage);
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_afe_dev_aux_voltages(AdbmsAfeStorage *afe, size_t afe_index, float voltage) {
  size_t afe_count = afe->settings->num_devices;

  if (afe_index >= afe_count) {
    LOG_DEBUG("Afe device does not exist, invalid index");
    return STATUS_CODE_INVALID_ARGS;
  }

  const uint16_t start = afe_index * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;
  const uint16_t end = start + ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;

  for (uint8_t aux_index = start; aux_index < end; ++aux_index) {
    adbms_afe_set_aux_voltage(afe, aux_index, voltage);
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_pack_cell_voltages(AdbmsAfeStorage *afe, float voltage) {
  uint8_t device_count = afe->settings->num_devices;
  for (uint8_t device = 0; device < device_count; ++device) {
    adbms_afe_set_afe_dev_cell_voltages(afe, device, voltage);
  }
  return STATUS_CODE_OK;
}
StatusCode adbms_afe_set_pack_aux_voltages(AdbmsAfeStorage *afe, float voltage) {
  uint8_t device_count = afe->settings->num_devices;
  for (uint8_t device = 0; device < device_count; ++device) {
    adbms_afe_set_afe_dev_aux_voltages(afe, device, voltage);
  }
  return STATUS_CODE_OK;
}

uint16_t adbms_afe_get_cell_voltage(AdbmsAfeStorage *afe, uint16_t index) {
  return afe->cell_voltages[index];
}

uint16_t adbms_afe_get_aux_voltage(AdbmsAfeStorage *afe, uint16_t index) {
  return afe->aux_voltages[index];
}

bool adbms_afe_get_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell) {
  bool discharge = 0;
  uint16_t cell_in_lookup = cell;
  uint16_t device_index = cell_in_lookup / ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  uint16_t device = afe->settings->num_devices - device_index - 1;
  uint16_t bit_position = cell_in_lookup % ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  if (bit_position < 12) {
    uint16_t bitset = afe->device_configs->devices[device].cfgA.discharge_bitset;
    discharge = (bitset >> bit_position) & 0x1;
  } else {
    uint16_t b_bit = bit_position - 12;
    uint16_t bitset = afe->device_configs->devices[device].cfgB.discharge_bitset;
    discharge = (bitset >> b_bit) & 0x1;
  }

  return discharge;
}
