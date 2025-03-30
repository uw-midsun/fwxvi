#include "ltc_afe.h"
#include <stddef.h>
#include <string.h>

// Will port in crc15 for now. Worth discussing in the future if we really need it tbh
// #include "crc15.h"
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

// Just to ensure we read only from voltage registers 
static const uint8_t s_voltage_reg[NUM_LTC_AFE_VOLTAGE_REGISTERS] = {
  [LTC_AFE_VOLTAGE_REGISTER_A] = LTC_AFE_REGISTER_CELL_VOLTAGE_A,
  [LTC_AFE_VOLTAGE_REGISTER_B] = LTC_AFE_REGISTER_CELL_VOLTAGE_B,
  [LTC_AFE_VOLTAGE_REGISTER_C] = LTC_AFE_REGISTER_CELL_VOLTAGE_C,
  [LTC_AFE_VOLTAGE_REGISTER_D] = LTC_AFE_REGISTER_CELL_VOLTAGE_D,
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

static StatusCode prv_read_register(LtcAfeStorage *afe, LtcAfeRegister reg, uint8_t * data, size_t len){
  if (reg > NUM_LTC_AFE_REGISTERS){
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t reg_cmd = s_read_reg_cmd[reg];
  uint8_t cmd[LTC6811_CMD_SIZE] = {0};  // Stores SPI command bytes

  prv_build_cmd(reg_cmd, cmd, LTC6811_CMD_SIZE);

  prv_wakeup_idle(afe);
  return spi_exchange(afe->settings.spi_port, cmd, LTC6811_CMD_SIZE, data, len);
}

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

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  // Check if arguments are valid

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
  return STATUS_CODE_UNIMPLEMENTED;
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
