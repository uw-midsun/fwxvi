/************************************************************************************************
 * @file   ltc_afe.c
 *
 * @brief  Source file for the LTC8611 AFE driver
 *
 * @date   2025-06-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <string.h>

/* Inter-component Headers */
#include "delay.h"
#include "log.h"
#include "status.h"

/* Intra-component Headers */
#include "ltc_afe.h"
#include "ltc_afe_crc15.h"
#include "ltc_afe_regs.h"

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

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *config){
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
  memset(afe, 0, sizeof(*afe));
  memcpy(afe->settings, config, sizeof(LtcAfeSettings));

  /* Calculate offset for cell result array due to some cells being disabled */
  s_calc_offsets(afe);

  /* Initialize 15-bit CRC lookup table to optimize packet error code (PEC) calculation */
  crc15_init_table();

  return ltc_afe_write_config(afe);
}

StatusCode ltc_afe_write_config(LtcAfeStorage *afe) {
    return STATUS_CODE_OK;  
}   


StatusCode ltc_afe_trigger_cell_conv(LtcAfeStorage *afe){
    LOG_DEBUG("ADC conversion for enabled cell voltages completed");
    return STATUS_CODE_OK;
}

StatusCode ltc_afe_trigger_aux_conv(LtcAfeStorage *afe, uint8_t device_cell){
    LOG_DEBUG("ADC conversion for enabled auxillary voltages completed");
    return STATUS_CODE_OK;
}

StatusCode ltc_afe_read_cells(LtcAfeStorage *afe){
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

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge){
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

StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle){
    LOG_DEBUG("Discharge PWM duty cycle set to %u", duty_cycle);
    return STATUS_CODE_OK; 
}


StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle){
    LtcAfeSettings *settings = afe->settings;
    return STATUS_CODE_OK; 
}

StatusCode ltc_afe_set_cell_voltage(LtcAfeStorage *afe, uint8_t cell_index, float voltage){
    size_t cell_count = afe->settings->num_cells; 
    if (cell_index >= cell_count){
        LOG_DEBUG("Invalid cell index number");
        return STATUS_CODE_INVALID_ARGS; 
    }
    
    afe->cell_voltages[cell_index] = voltage; 
    return STATUS_CODE_OK; 
}
StatusCode ltc_afe_set_aux_voltage(LtcAfeStorage *afe, uint8_t aux_index, float voltage){
    size_t thermistors = afe->settings->num_thermistors; 
    if (aux_index >= thermistors){
        LOG_DEBUG("Invalid aux index number");
        return STATUS_CODE_INVALID_ARGS; 
    }

    afe->aux_voltages[aux_index] = voltage; 
    return STATUS_CODE_OK; 
}

StatusCode ltc_afe_set_afe_cell_voltages(LtcAfeStorage *afe, uint8_t afe_index, float voltage){
    size_t afe_count = afe->settings->num_devices; 
    
    if (afe_index >= afe_count){
        LOG_DEBUG("Afe device does not exist, invalid index"); 
        return STATUS_CODE_INVALID_ARGS; 
    }

    uint8_t cell_count = afe->settings->num_cells; 
    for (uint8_t cell_index = 0; cell_index < cell_count; ++cell_index){
        uint8_t base = afe_index * cell_count; 
        ltc_afe_set_cell_voltage(afe, base + cell_index, voltage); 
    }

    return STATUS_CODE_OK;
}

StatusCode ltc_afe_set_afe_aux_voltages(LtcAfeStorage *afe, uint8_t afe_index, float voltage){
    size_t afe_count = afe->settings->num_devices; 
    
    if (afe_index >= afe_count){
        LOG_DEBUG("Afe device does not exist, invalid index"); 
        return STATUS_CODE_INVALID_ARGS; 
    }

    uint8_t aux_count = afe->settings->num_thermistors;
    for (uint8_t aux_index = 0; aux_index < aux_count; ++aux_index){
        uint8_t base = afe_index * aux_count; 
        ltc_afe_set_aux_voltage(afe, base + aux_index, voltage); 
    }

    return STATUS_CODE_OK;
}

StatusCode ltc_afe_set_pack_cell_voltages(LtcAfeStorage *afe, float voltage){
    uint8_t device_count = afe->settings->num_devices; 
    for(uint8_t device = 0; device < device_count; ++device){
        ltc_afe_set_afe_cell_voltages(afe, device, voltage); 
    }
    return STATUS_CODE_OK; 
}
StatusCode ltc_afe_set_pack_aux_voltages(LtcAfeStorage *afe, float voltage){
    uint8_t device_count = afe->settings->num_devices; 
    for(uint8_t device = 0; device < device_count; ++device){
        ltc_afe_set_afe_aux_voltages(afe, device, voltage); 
    }
    return STATUS_CODE_OK; 
}