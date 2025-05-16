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

static StatusCode s_build_cmd(uint16_t command, uint8_t *cmd, size_t len);

/**
 * @brief Commands for reading registers + STCOMM
 * @note  Table 38 (p 59)
 **/
static const uint16_t s_read_reg_cmd[NUM_LTC_AFE_REGISTERS] = {
  [LTC_AFE_REGISTER_CONFIG] = LTC6811_RDCFG_RESERVED,         [LTC_AFE_REGISTER_CELL_VOLTAGE_A] = LTC6811_RDCVA_RESERVED, [LTC_AFE_REGISTER_CELL_VOLTAGE_B] = LTC6811_RDCVB_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_C] = LTC6811_RDCVC_RESERVED, [LTC_AFE_REGISTER_CELL_VOLTAGE_D] = LTC6811_RDCVD_RESERVED, [LTC_AFE_REGISTER_AUX_A] = LTC6811_RDAUXA_RESERVED,
  [LTC_AFE_REGISTER_AUX_B] = LTC6811_RDAUXB_RESERVED,         [LTC_AFE_REGISTER_STATUS_A] = LTC6811_RDSTATA_RESERVED,     [LTC_AFE_REGISTER_STATUS_B] = LTC6811_RDSTATB_RESERVED,
  [LTC_AFE_REGISTER_READ_COMM] = LTC6811_RDCOMM_RESERVED,     [LTC_AFE_REGISTER_START_COMM] = LTC6811_STCOMM_RESERVED
};

/* p. 52 - Daisy chain wakeup method 2 - pair of long -1, +1 for each device */
static void s_wakeup_idle(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = afe->settings;

  for (size_t i = 0; i < settings->num_devices; i++) {
    gpio_set_state(&settings->spi_settings.cs, GPIO_STATE_LOW);
    gpio_set_state(&settings->spi_settings.cs, GPIO_STATE_HIGH);
    delay_ms(1);
  }
}

/* Read data from register and store it in devices_data */
static StatusCode s_read_register(LtcAfeStorage *afe, LtcAfeRegister reg, uint8_t *devices_data, size_t len) {
  if (reg >= NUM_LTC_AFE_REGISTERS || devices_data == NULL || len == 0) {
    LOG_DEBUG("Invalid arguments: Can't read register ");
    return STATUS_CODE_INVALID_ARGS;
  }

  LtcAfeSettings *settings = afe->settings;

  uint16_t reg_cmd = s_read_reg_cmd[reg];
  uint8_t cmd[LTC6811_CMD_SIZE] = { 0 }; /* Stores SPI command byte */
  s_build_cmd(reg_cmd, cmd, LTC6811_CMD_SIZE);

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, LTC6811_CMD_SIZE, devices_data, len);
}

/* Split command into 8 byte chunks and add PEC to full command */
static StatusCode s_build_cmd(uint16_t command, uint8_t *cmd, size_t len) {
  if (len != LTC6811_CMD_SIZE) {
    LOG_DEBUG("Command length is not the correct size");
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Layout of command: CMD0, CMD1, PEC0, PEC1 */
  cmd[0] = (uint8_t)(command >> 8);
  cmd[1] = (uint8_t)(command & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec & 0xFF);

  return STATUS_CODE_OK;
}

/* Used as a wrapper internally, since in some functions we don't want the default gpio_pins */
static StatusCode s_write_config(LtcAfeStorage *afe, uint8_t gpio_enable_pins) {
  LtcAfeSettings *settings = afe->settings;
  LtcAfeWriteConfigPacket *config_packet = afe->device_configs;

  s_build_cmd(LTC6811_WRCFG_RESERVED, config_packet->wrcfg, SIZEOF_ARRAY(config_packet->wrcfg));

  /*
   * Each set of CFGR registers (containing config data) is shifted through the chain of devices,
   * so the first set sent reaches the last device (like a shift register).
   * Therefore, we send config settings starting with the last slave in the stack.
   */
  for (uint8_t curr_device = 0; curr_device < settings->num_devices; curr_device++) {
    /* Shift 3 since CFGR0 bitfield uses the last 5 bits (See Table 40 on p. 62) */
    config_packet->devices[curr_device].reg.gpio = (gpio_enable_pins >> 3);

    uint16_t cfgr_pec = crc15_calculate((uint8_t *)&config_packet->devices[curr_device].reg, 6);
    config_packet->devices[curr_device].pec = cfgr_pec >> 8 | cfgr_pec << 8; /* Swap endianness */
  }

  size_t len = SIZEOF_LTC_AFE_WRITE_CONFIG_PACKET(settings->num_devices);
  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, (uint8_t *)&config_packet, len, NULL, 0);
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
  memset(afe, 0, sizeof(*afe));
  memcpy(afe->settings, config, sizeof(LtcAfeSettings));

  /* Set up SPI communication based on provided settings */
  SpiSettings spi_config = afe->settings->spi_settings;

  spi_init(config->spi_port, &spi_config);

  /* Calculate offset for cell result array due to some cells being disabled */
  s_calc_offsets(afe);

  /* Initialize 15-bit CRC lookup table to optimize packet error code (PEC) calculation */
  crc15_init_table();

  /* Set same duty cycle for all cells in the AFE system */
  StatusCode status = ltc_afe_set_discharge_pwm_cycle(afe, LTC6811_PWMC_DC_100);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Read register failed");
    return status;
  }

  /*
   * Write configuration settings to AFE.
   * GPIO pins are configured with pull-down off: GPIO1 as analog input and GPIO3-5 for SPI
   */
  return ltc_afe_write_config(afe);
}

StatusCode ltc_afe_write_config(LtcAfeStorage *afe) {
  uint8_t gpio_bits = LTC6811_GPIO1_PD_OFF | LTC6811_GPIO3_PD_OFF | LTC6811_GPIO4_PD_OFF | LTC6811_GPIO5_PD_OFF;

  LtcAfeSettings *settings = afe->settings;

  for (uint8_t curr_device = 0; curr_device < settings->num_devices; curr_device++) {
    LtcAfeConfigRegisterData *reg = &afe->device_configs->devices[curr_device].reg;

    reg->discharge_bitset = 0;
    reg->discharge_timeout = LTC_AFE_DISCHARGE_TIMEOUT_30_S;

    reg->adcopt = ((settings->adc_mode + 1) > 3);
    reg->dten = true;
    reg->refon = 0;

    reg->undervoltage = 0;
    reg->overvoltage = 0;
  }

  return s_write_config(afe, gpio_bits);
}

StatusCode ltc_afe_trigger_cell_conv(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = afe->settings;

  /* See Table 39 (p. 61) for the MD[1:0] command bit description and values */
  uint8_t md_cmd_bits = (uint8_t)((settings->adc_mode) % (NUM_LTC_AFE_ADC_MODES / 2));

  /* ADVC Command Code (see Table 38 on p. 60) */
  uint16_t adcv = LTC6811_ADCV_RESERVED | LTC6811_ADCV_DISCHARGE_NOT_PERMITTED | LTC6811_CNVT_CELL_ALL | (md_cmd_bits << 7);

  uint8_t cmd[LTC6811_CMD_SIZE] = { 0 };
  s_build_cmd(adcv, cmd, LTC6811_CMD_SIZE);

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, LTC6811_CMD_SIZE, NULL, 0);
}

/* Start GPIOs ADC conversion and poll status for XXXXX */
StatusCode ltc_afe_trigger_aux_conv(LtcAfeStorage *afe, uint8_t thermistor) {
  LtcAfeSettings *settings = afe->settings;

  /* Left=shift by 3 since CFGR0 bitfield uses the last 5 bits (See Table 40 on p. 62) */
  uint8_t gpio_bits = (thermistor << 3) | LTC6811_GPIO4_PD_OFF;
  s_write_config(afe, gpio_bits);

  /* See Table 39 (p. 61) for the MD[1:0] command bit description and values */
  uint8_t md_cmd_bits = (uint8_t)((settings->adc_mode) % (NUM_LTC_AFE_ADC_MODES / 2));
  /* ADAX Command Code (see Table 38 on pg. 60) */
  uint16_t adax = LTC6811_ADAX_RESERVED | LTC6811_ADAX_GPIO4 | (md_cmd_bits << 7);

  uint8_t cmd[LTC6811_CMD_SIZE] = { 0 };
  s_build_cmd(adax, cmd, LTC6811_CMD_SIZE);

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, LTC6811_CMD_SIZE, NULL, 0);
}

StatusCode ltc_afe_read_cells(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = afe->settings;

  for (LtcAfeVoltageRegister v_reg_group = LTC_AFE_VOLTAGE_REGISTER_A; v_reg_group < NUM_LTC_AFE_VOLTAGE_REGISTERS; ++v_reg_group) {
    /* Max devices macro used because value must be known at runtime */
    LtcAfeVoltageData devices_data[LTC_AFE_MAX_DEVICES] = { 0 };

    size_t len = sizeof(LtcAfeVoltageData) * settings->num_devices;
    StatusCode status = s_read_register(afe, (LtcAfeRegister)v_reg_group, (uint8_t *)devices_data, len);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Read register failed");
      return status;
    }

    /* Loop through the number of AFE devices connected for each voltage group */
    for (uint8_t device = 0; device < settings->num_devices; ++device) {
      uint16_t received_pec = devices_data[device].pec >> 8 | devices_data[device].pec << 8;  // Swap endianness
      uint16_t data_pec = crc15_calculate((uint8_t *)&devices_data[device], 6);

      /* Check if there is an error in the data */
      if (data_pec != received_pec) {
        LOG_DEBUG("Communication Failed with device: %d\n\r", device);
        LOG_DEBUG("RECEIVED_PEC: %d\n\r", received_pec);
        LOG_DEBUG("DATA_PEC: %d\n\r", data_pec);
        LOG_DEBUG("Voltage: %d %d %d\n\r", devices_data[device].reg.voltages[0], devices_data[device].reg.voltages[1], devices_data[device].reg.voltages[2]);
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
  LtcAfeSettings *settings = afe->settings;
  LtcAfeAuxData devices_reg_data[LTC_AFE_MAX_DEVICES] = { 0 };

  size_t len = settings->num_devices * sizeof(LtcAfeAuxData);

  StatusCode status = s_read_register(afe, LTC_AFE_REGISTER_AUX_B, (uint8_t *)devices_reg_data, len);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Read register failed");
    return status;
  }

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

    uint16_t received_pec = devices_reg_data[device].pec >> 8 | devices_reg_data[device].pec << 8;
    uint16_t data_pec = crc15_calculate((uint8_t *)&devices_reg_data[device], 6);

    /* Check if there is an error in the data */
    if (received_pec != data_pec) {
      LOG_DEBUG("Communication Failed with device: %d\n\r", device);
      LOG_DEBUG("RECEIVED_PEC: %d\n\r", received_pec);
      LOG_DEBUG("DATA_PEC: %d\n\r", data_pec);
      return STATUS_CODE_INTERNAL_ERROR;
    }

    uint16_t voltage = devices_reg_data[device].reg.voltages[0];

    uint16_t index = device * LTC_AFE_MAX_THERMISTORS_PER_DEVICE + device_cell;
    afe->aux_voltages[index] = voltage;
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
  LtcAfeSettings *settings = afe->settings;

  /**
   * Initialize a buffer to store the PWM configuration command, while also accounting for
   * the number of registers to write to (6) for each AFE, which is not known at compile time.
   */
  uint8_t cmd_with_pwm[LTC6811_CMD_SIZE + (LTC8611_NUM_PWM_REGS * LTC_AFE_MAX_DEVICES)] = { 0 };

  /* Prepare command to be sent, disregarding the PWM configuration data */
  s_build_cmd(LTC6811_WRPWM_RESERVED, cmd_with_pwm, 4);

  /* Pack 4-bit duty cycle configuration for two PWM channels in an 8-bit PWM register */
  uint8_t packed_duty_cycle = (duty_cycle << 4) | duty_cycle;

  /* Set all 6 PWM registers to the same configuration for each AFE */
  for (uint8_t device = 0; device < settings->num_devices; device++) {
    /**
     * Since each AFE has 12 PWM channels and each PWM register stores two 4-bit PWM values in a byte,
     * there are 6 registers to iterate through and 2 PWM channels are configured at a time.
     */
    for (int pwm_reg = 0; pwm_reg < LTC8611_NUM_PWM_REGS; pwm_reg++) {
      /* Set both of the 2 PWM channels in the PWM register to the given configuration */
      cmd_with_pwm[LTC6811_CMD_SIZE + (device * LTC8611_NUM_PWM_REGS) + pwm_reg] = packed_duty_cycle;
    }
  }

  size_t cmd_with_pwm_len = LTC6811_CMD_SIZE + (LTC8611_NUM_PWM_REGS * settings->num_devices);
  /* Wake up the devices if they are idle before sending the command */
  s_wakeup_idle(afe);

  /* Send PWM configuration command to all AFEs */
  return spi_exchange(settings->spi_port, cmd_with_pwm, cmd_with_pwm_len, NULL, 0);
}
