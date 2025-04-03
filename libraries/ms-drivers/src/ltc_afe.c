#include "ltc_afe.h"
#include <stddef.h>
#include <string.h>

#include "ltc_afe_crc15.h"
#include "delay.h"
#include "log.h"
#include "ltc_afe_regs.h"

/* Table 38 (p 59)
 * Commands for reading registers + STCOMM */
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


/* p. 52 - Daisy chain wakeup method 2 - pair of long -1, +1 for each device */
static void prv_wakeup_idle(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = &afe->settings;
  for (size_t i = 0; i < settings->num_devices; i++) {
    gpio_set_state(&settings->cs, GPIO_STATE_LOW);
    gpio_set_state(&settings->cs, GPIO_STATE_HIGH);
    delay_ms(1);
  }
}

/* Read data from register and store it in devices_data */
static StatusCode prv_read_register(LtcAfeStorage *afe, LtcAfeRegister reg, uint8_t *devices_data, size_t len){
  if (reg >= NUM_LTC_AFE_REGISTERS || devices_data == NULL || len == 0){
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  LtcAfeSettings *settings = &afe->settings;

  uint16_t reg_cmd = s_read_reg_cmd[reg];
  uint8_t cmd[LTC6811_CMD_SIZE] = {0};  /* Stores SPI command byte */ 
  prv_build_cmd(reg_cmd, cmd, LTC6811_CMD_SIZE);

  prv_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, LTC6811_CMD_SIZE, devices_data, len);
}

/* Split command into 8 byte chunks and add PEC to full command */ 
static StatusCode prv_build_cmd(uint16_t command, uint8_t *cmd, size_t len){
  if (len != LTC6811_CMD_SIZE){
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  cmd[0] = (uint8_t) (command >> 8);
  cmd[1] = (uint8_t) (command & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  cmd[2] = (uint8_t) (cmd_pec >> 8);
  cmd[3] = (uint8_t) (cmd_pec & 0xFF);

  /* Layout of command: CMD0, CMD1, PEC0, PEC1 */ 

  return STATUS_CODE_OK;
}

static StatusCode prv_write_config(LtcAfeStorage *afe, uint8_t gpio_enable_pins){
  LtcAfeSettings *settings = &afe->settings;

}

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *config) {
  // Check if arguments are valid
  if (config->num_devices > LTC_AFE_MAX_DEVICES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, 
                      "AFE: Configured device count exceeds user-defined limit. Update LTC_AFE_MAX_DEVICES if necessary.");
  }
  if (config->num_cells > LTC_AFE_MAX_CELLS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, 
                      "AFE: Configured cell count exceeds device limitations.");
  }
  if (config->num_thermistors > LTC_AFE_MAX_THERMISTORS) {
    return status_msg(STATUS_CODE_INVALID_ARGS,
                      "AFE: Configured thermistor count exceeds limitations.");
  }

  // Initialize memory
  memset(afe, 0, sizeof(*afe));  // Reset value of all afe struct members to 0
  // Copy values of config struct members to address of the settings struct within afe
  memcpy(&afe->settings, config, sizeof(afe->settings));  

  // Calculate offsets

  // Initialize CRC stuff??? (look at fwxv)

  // Initialize SPI settings

  // Configure GPIO

  // Set PWM cycle settings

  // Actually write configuration settings to AFE fr fr
  
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_write_config(LtcAfeStorage *afe) {
  uint8_t gpio_bits = 
    LTC6811_GPIO1_PD_OFF | LTC6811_GPIO3_PD_OFF | LTC6811_GPIO4_PD_OFF | LTC6811_GPIO5_PD_OFF;
  

  return STATUS_CODE_OK;
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

    /* Max devices macro used because value must be known at runtime */
    LtcAfeVoltageData devices_data[LTC_AFE_MAX_DEVICES] = { 0 };

    size_t len = sizeof(LtcAfeVoltageData) * settings->num_devices;
    status_ok_or_return(prv_read_register(afe, v_reg_group, (uint8_t *)devices_data, len));

    /* Loop through the number of AFE devices connected for each voltage group */
    for (uint8_t device = 0; device < settings->num_devices; ++device) {
      uint16_t received_pec = devices_data[device].pec >> 8 | devices_data[device].pec << 8; // Swap endianness
      uint16_t data_pec = crc15_calculate((uint8_t *)&devices_data[device], 6);

      /* Check if there is an error in the data */
      if (data_pec != received_pec) {
        LOG_DEBUG("Communication Failed with device: %d\n\r", device);
        LOG_DEBUG("RECEIVED_PEC: %d\n\r", received_pec);
        LOG_DEBUG("DATA_PEC: %d\n\r", data_pec);
        LOG_DEBUG("Voltage: %d %d %d\n\r", 
                  devices_data[device].reg.voltages[0],
                  devices_data[device].reg.voltages[1],
                  devices_data[device].reg.voltages[2]);
        return STATUS_CODE_INTERNAL_ERROR;
      }

      /* Loop through voltages in each register in the register group */
      for (uint16_t cell = 0; cell < LTC6811_CELLS_IN_REG; ++cell) {
        uint16_t voltage = devices_data[device].reg.voltages[cell];

        /* Determine index of the battery cell */
        uint16_t device_cell = cell + (v_reg_group * LTC6811_CELLS_IN_REG);
        uint16_t index = device_cell + (device * LTC_AFE_MAX_CELLS_PER_DEVICE);

        /* Store voltage if cell status is enabled */
        if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
          afe->cell_voltages[afe->cell_result_lookup[index]] = voltage;
        }
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode ltc_afe_read_aux(LtcAfeStorage *afe, uint8_t device_cell) {
  LtcAfeSettings *settings = &afe->settings;
  LtcAfeAuxData devices_reg_data[LTC_AFE_MAX_DEVICES] = { 0 };

  size_t len = settings->num_devices * sizeof(LtcAfeAuxData);
  status_ok_or_return(prv_read_register(afe, LTC_AFE_REGISTER_AUX_B, (uint8_t *)devices_reg_data, len));

  /* Loop through devices */
  for (uint16_t device = 0; device < settings->num_devices; ++device) {
    uint16_t received_pec = devices_reg_data[device].pec >> 8 | devices_reg_data[device].pec << 8;
    uint16_t data_pec = crc15_calculate((uint8_t *)&devices_reg_data[device], 6);

    /* Check if there is an error in the data */
    if (received_pec != data_pec) {
      LOG_DEBUG("Communication Failed with device: %d\n\r", device);
      LOG_DEBUG("RECEIVED_PEC: %d\n\r", received_pec);
      LOG_DEBUG("DATA_PEC: %d\n\r", data_pec);
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }

    /* data will be stored in voltages member in LtcAfeRegisterData struct in the form:
     * {GPIO4, AUXB2, AUXB3, PEC}
     * we only care about GPIO4 and the PEC */
    uint16_t voltage = devices_reg_data[device].reg.voltages[0];

    /* If input enabled - store result */ 
    if ((settings->aux_bitset[device] >> device_cell) & 0x1) {
      uint16_t index = device * LTC_AFE_MAX_THERMISTORS_PER_DEVICE + device_cell;
      afe->aux_voltages[index] = voltage;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle) {
  return STATUS_CODE_UNIMPLEMENTED;
}
