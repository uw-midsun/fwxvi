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
  LtcAfeWriteConfigPacket config_packet = { 0 };

  prv_build_cmd(LTC6811_WRCFG_RESERVED, config_packet.wrcfg, SIZEOF_ARRAY(config_packet.wrcfg));

  /*
   * Each set of CFGR registers (containing config data) is shifted through the chain of devices,
   * so the first set sent reaches the last device (like a shift register).
   * Therefore, we send config settings starting with the last slave in the stack.
  */
  for (uint8_t curr_device = 0; curr_device < settings->num_devices; curr_device++) {
    uint16_t undervoltage = 0;
    uint16_t overvoltage = 0;

    config_packet.devices[curr_device].reg.discharge_bitset = afe->discharge_bitset[curr_device];
    config_packet.devices[curr_device].reg.discharge_timeout = LTC_AFE_DISCHARGE_TIMEOUT_30_S;

    config_packet.devices[curr_device].reg.adcopt = ((settings->adc_mode + 1) > 3);
    config_packet.devices[curr_device].reg.dten = true;

    config_packet.devices[curr_device].reg.undervoltage = undervoltage;
    config_packet.devices[curr_device].reg.overvoltage = overvoltage;

    /* Shift 3 since bitfield uses the last 5 bits */
    config_packet.devices[curr_device].reg.gpio = (gpio_enable_pins >> 3);

    uint16_t cfgr_pec = crc15_calculate((uint8_t *)&config_packet.devices[curr_device].reg, 6);
    config_packet.devices[curr_device].pec = cfgr_pec >> 8 | cfgr_pec << 8; /* Swap endianness */
  }

  size_t len = SIZEOF_LTC_AFE_WRITE_CONFIG_PACKET(settings->num_devices);
  prv_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, (uint8_t *)&config_packet, len, NULL, 0);


}

static void prv_calc_offsets(LtcAfeStorage *afe) {
  // Our goal is to populate result arrays as if the ignored inputs don't exist. This requires
  // converting the actual LTC6811 cell index to some potentially smaller result index.
  //
  // Since we access the same register across multiple devices, we can't just keep a counter and
  // increment it for each new value we get during register access. Instead, we precompute each
  // input's corresponding result index. Inputs that are ignored will not be copied into the result
  // array.
  //
  // Similarly, we do the opposite mapping for discharge.

  LtcAfeSettings *settings = &afe->settings;
  size_t enabled_cell_index = 0;

  for (size_t device = 0; device < settings->num_devices; device++) {
    size_t device_offset = device * LTC_AFE_MAX_CELLS_PER_DEVICE;  // Pre-compute offset
    uint16_t bitmask = settings->cell_bitset[device];  // Extract bitmask once, showing enabled cells

    for (size_t device_cell = 0; device_cell < LTC_AFE_MAX_CELLS_PER_DEVICE; device_cell++) {
      size_t raw_cell_index = device_offset + device_cell;  // Absolute index of cell in current device across all devices

      // Check if current cell is enabled through the given bitmask
      if ((bitmask >> device_cell) & 0x1) {
        // Cell input enabled - store the index that this input should be stored in
        // when copying to the result array and the opposite for discharge
        afe->discharge_cell_lookup[enabled_cell_index] = raw_cell_index;
        afe->cell_result_lookup[raw_cell_index] = enabled_cell_index++; 
      }
    }
  }
}

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *config) {
  /* Validate configuration settings based on device limitations */
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

  /* Initialize memory */
  memset(afe, 0, sizeof(*afe));  // Reset value of all afe struct members to 0
  memcpy(&afe->settings, config, sizeof(afe->settings));  // Copy values of config struct members to address of the settings struct within afe

  /* Initialize SPI */
  SpiSettings spi_config = {
    .baudrate = config->spi_baudrate,
    .mode = SPI_MODE_3,
    .mosi = config->mosi,
    .miso = config->miso,
    .sclk = config->sclk,
    .cs = config->cs,
  };
  spi_init(config->spi_port, &spi_config);

  // Calculate offsets
  prv_calc_offsets(afe);

  // Initialize 15-bit CRC lookup table to optimize packet error code (PEC) calculation
  crc15_init_table();

  /* Configure GPIO pins with pull-down off: GPIO1 as analog input and GPIO3-5 for SPI */
  uint8_t gpio_bitmask =
      LTC6811_GPIO1_PD_OFF | 
      LTC6811_GPIO3_PD_OFF | 
      LTC6811_GPIO4_PD_OFF | 
      LTC6811_GPIO5_PD_OFF;

  // Set PWM cycle settings
  status_ok_or_return(ltc_afe_set_discharge_pwm_cycle(afe, LTC6811_PWMC_DC_100));

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

// Sets the duty cycle to the same value for all cells on all afes
StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle) {
  LtcAfeSettings *settings = &afe->settings;

  uint8_t cmd[4 + (6 * 3)] = { 0 };
  prv_build_cmd(LTC6811_WRPWM_RESERVED, cmd, 4);

  return STATUS_CODE_UNIMPLEMENTED;
}
