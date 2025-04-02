#include "ltc_afe.h"
#include <stddef.h>
#include <string.h>

// Will port in crc15 for now. Worth discussing in the future if we really need it tbh
#include "ltc_afe_crc15.h"
#include "delay.h"
#include "log.h"
#include "ltc_afe_regs.h"

// Table 38 (p 59)
// Commands for reading registers + STCOMM
static const uint16_t s_read_reg_cmd[NUM_LTC_AFE_REGISTERS] = {
  [LTC_AFE_REGISTER_CONFIG] = LTC6811_RDCFG_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_A] = LTC6811_RDCVA_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_B] = LTC6811_RDCVB_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_C] = LTC6811_RDCVC_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_D] = LTC6811_RDCVD_RESERVED,
  [LTC_AFE_REGISTER_AUX_A] = LTC6811_RDAUXA_RESERVED,
  [LTC_AFE_REGISTER_AUX_B] = LTC6811_RDAUXB_RESERVED,
  [LTC_AFE_REGISTER_STATUS_A] = LTC6811_RDSTATA_RESERVED,
  [LTC_AFE_REGISTER_STATUS_B] = LTC6811_RDSTATB_RESERVED,
  [LTC_AFE_REGISTER_READ_COMM] = LTC6811_RDCOMM_RESERVED,
  [LTC_AFE_REGISTER_START_COMM] = LTC6811_STCOMM_RESERVED
};


// p. 52 - Daisy chain wakeup method 2 - pair of long -1, +1 for each device
static void prv_wakeup_idle(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = &afe->settings;
  for (size_t i = 0; i < settings->num_devices; i++) {
    gpio_set_state(&settings->cs, GPIO_STATE_LOW);
    gpio_set_state(&settings->cs, GPIO_STATE_HIGH);
    delay_ms(1);
  }
}

static StatusCode prv_read_register(LtcAfeStorage *afe, LtcAfeRegister reg, uint8_t *all_devices_data, size_t len){
  if (reg >= NUM_LTC_AFE_REGISTERS || all_devices_data == NULL || len == 0){
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t reg_cmd = s_read_reg_cmd[reg];
  uint8_t cmd[LTC6811_CMD_SIZE] = {0};  // Stores SPI command bytes

  prv_build_cmd(reg_cmd, cmd, LTC6811_CMD_SIZE);

  prv_wakeup_idle(afe);
  return spi_exchange(afe->settings.spi_port, cmd, LTC6811_CMD_SIZE, all_devices_data, len);
}

// Split command into 8 byte chunks and add PEC to full command
static StatusCode prv_build_cmd(uint16_t command, uint8_t *cmd, size_t len){
  if (len != LTC6811_CMD_SIZE){
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  cmd[0] = (uint8_t) (command >> 8);
  cmd[1] = (uint8_t) (command & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  cmd[2] = (uint8_t) (cmd_pec >> 8);
  cmd[3] = (uint8_t) (cmd_pec & 0xFF);

  // Layout of command: 0xCMD[0]\0xCMD[1]\0xCMD[2]\0xCMD[3]

  return STATUS_CODE_OK;
}

// read from a voltage register
static StatusCode prv_read_voltage(LtcAfeStorage *afe, LtcAfeVoltageRegister reg, LtcAfeVoltageData *all_devices_data) {
  if (reg > NUM_LTC_AFE_VOLTAGE_REGISTERS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  size_t len = sizeof(LtcAfeVoltageData) * afe->settings.num_devices;
  return prv_read_register(afe, reg, (uint8_t *)all_devices_data, len);
}

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  // Check if arguments are valid
  if (settings->num_devices > LTC_AFE_MAX_DEVICES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "AFE: Configured device count exceeds user-defined limit. Update LTC_AFE_MAX_DEVICES if necessary.");
  }
  if (settings->num_cells > LTC_AFE_MAX_CELLS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "AFE: Configured cell count exceeds device limitations.");
  }
  if (settings->num_thermistors > LTC_AFE_MAX_THERMISTORS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "AFE: Configured thermistor count exceeds limitations.");
  }

  // Initialize memory

  // Calculate offsets

  // Initialize CRC stuff??? (look at fwxv)

  // Initialize SPI settings

  // Configure GPIO

  // Set PWM cycle settings

  // Actually write configuration settings to AFE fr fr
  
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_write_config(LtcAfeStorage *afe) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_trigger_cell_conv(LtcAfeStorage *afe) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_trigger_aux_conv(LtcAfeStorage *afe, uint8_t device_cell) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_read_cells(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = &afe->settings;

  for (LtcAfeVoltageRegister v_reg_group = LTC_AFE_VOLTAGE_REGISTER_A;
       v_reg_group < NUM_LTC_AFE_VOLTAGE_REGISTERS;
       ++v_reg_group) {
    // Max devices macro used because value must be known at runtime
    LtcAfeVoltageData afe_device[LTC_AFE_MAX_DEVICES] = { 0 };
    status_ok_or_return(prv_read_voltage(afe, v_reg_group, afe_device));

    // Loop through the number of AFE devices connected for each voltage group
    for (uint8_t device = 0; device < settings->num_devices; ++device) {
      uint16_t received_pec = afe_device[device].pec >> 8 | afe_device[device].pec << 8; // Swap endianness
      uint16_t data_pec = crc15_calculate((uint8_t *)&afe_device[device], 6);

      // Check if there is an error in the data
      if (data_pec != received_pec) {
        LOG_DEBUG("Communication Failed with device: %d\n\r", device);
        LOG_DEBUG("RECEIVED_PEC: %d\n\r", received_pec);
        LOG_DEBUG("DATA_PEC: %d\n\r", data_pec);
        LOG_DEBUG("Voltage: %d %d %d\n\r", 
                  afe_device[device].reg.voltages[0],
                  afe_device[device].reg.voltages[1],
                  afe_device[device].reg.voltages[2]);
        return STATUS_CODE_INTERNAL_ERROR;
      }

      // Loop through voltages in each register in the register group
      for (uint16_t cell = 0; cell < LTC6811_CELLS_IN_REG; ++cell) {
        uint16_t voltage = afe_device[device].reg.voltages[cell];

        // Determine index of the battery cell
        uint16_t device_cell = cell + (v_reg_group * LTC6811_CELLS_IN_REG);
        uint16_t index = device_cell + (device * LTC_AFE_MAX_CELLS_PER_DEVICE);

        // Store voltage if cell status is enabled
        if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
          afe->cell_voltages[afe->cell_result_lookup[index]] = voltage;
        }
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode ltc_afe_read_aux(LtcAfeStorage *afe, uint8_t device_cell) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle) {
  return STATUS_CODE_UNIMPLEMENTED;
}
