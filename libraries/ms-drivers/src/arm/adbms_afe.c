/************************************************************************************************
 * @file   adbms_afe.c
 *
 * @brief  Source file for the ADBMS1818 AFE driver
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
#include "adbms_afe.h"
#include "adbms_afe_crc15.h"
#include "adbms_afe_regs.h"

static StatusCode s_build_cmd(uint16_t command, uint8_t *cmd, size_t len);

/**
 * @brief Commands for reading registers + STCOMM
 * @note  Table 38 (p 59)
 **/
static const uint16_t s_read_reg_cmd[NUM_ADBMS_AFE_REGISTERS] = {
  [ADBMS_AFE_REGISTER_CONFIG_A] = ADBMS1818_RDCFGA_RESERVED,  [ADBMS_AFE_REGISTER_CELL_VOLTAGE_A] = ADBMS1818_RDCVA_RESERVED, [ADBMS_AFE_REGISTER_CELL_VOLTAGE_B] = ADBMS1818_RDCVB_RESERVED,
  [ADBMS_AFE_REGISTER_CONFIG_B] = ADBMS1818_RDCFGB_RESERVED,  [ADBMS_AFE_REGISTER_CELL_VOLTAGE_C] = ADBMS1818_RDCVC_RESERVED, [ADBMS_AFE_REGISTER_CELL_VOLTAGE_D] = ADBMS1818_RDCVD_RESERVED,
  [ADBMS_AFE_REGISTER_AUX_A] = ADBMS1818_RDAUXA_RESERVED,     [ADBMS_AFE_REGISTER_AUX_B] = ADBMS1818_RDAUXB_RESERVED,         [ADBMS_AFE_REGISTER_STATUS_A] = ADBMS1818_RDSTATA_RESERVED,
  [ADBMS_AFE_REGISTER_STATUS_B] = ADBMS1818_RDSTATB_RESERVED, [ADBMS_AFE_REGISTER_READ_COMM] = ADBMS1818_RDCOMM_RESERVED,     [ADBMS_AFE_REGISTER_START_COMM] = ADBMS1818_STCOMM_RESERVED
};

/* p. 56-57 - Daisy chain wakeup method 2 - pair of long -1, +1 for each device */
static void s_wakeup_idle(AdbmsAfeStorage *afe) {
  AdbmsAfeSettings *settings = afe->settings;

  for (size_t i = 0; i < settings->num_devices; i++) {
    gpio_set_state(&settings->spi_settings.cs, GPIO_STATE_LOW);
    gpio_set_state(&settings->spi_settings.cs, GPIO_STATE_HIGH);
    delay_ms(1);
  }
}

/* Read data from register and store it in devices_data */
static StatusCode s_read_register(AdbmsAfeStorage *afe, AdbmsAfeRegister reg, uint8_t *devices_data, size_t len) {
  if (reg >= NUM_ADBMS_AFE_REGISTERS || devices_data == NULL || len == 0) {
    LOG_DEBUG("Invalid arguments: Can't read register ");
    return STATUS_CODE_INVALID_ARGS;
  }

  AdbmsAfeSettings *settings = afe->settings;

  uint16_t reg_cmd = s_read_reg_cmd[reg];
  uint8_t cmd[ADBMS1818_CMD_SIZE] = { 0 }; /* Stores SPI command byte */
  s_build_cmd(reg_cmd, cmd, ADBMS1818_CMD_SIZE);

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, ADBMS1818_CMD_SIZE, devices_data, len);
}

/* Split command into 8 byte chunks and add PEC to full command */
static StatusCode s_build_cmd(uint16_t command, uint8_t *cmd, size_t len) {
  if (len != ADBMS1818_CMD_SIZE) {
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
static StatusCode s_write_config(AdbmsAfeStorage *afe, uint8_t *gpio_enable_pins) {
  AdbmsAfeSettings *settings = afe->settings;
  AdbmsAfeWriteConfigPacket *config_packet = afe->device_configs;

  s_build_cmd(ADBMS1818_WRCFGA_RESERVED, config_packet->wrcfg, SIZEOF_ARRAY(config_packet->wrcfg));
  s_build_cmd(ADBMS1818_WRCFGB_RESERVED, config_packet->wrcfg, SIZEOF_ARRAY(config_packet->wrcfg));

  /*
   * Each set of CFGR registers (containing config data) is shifted through the chain of devices,
   * so the first set sent reaches the last device (like a shift register).
   * Therefore, we send config settings starting with the last slave in the stack.
   */
  for (uint8_t curr_device = 0; curr_device < settings->num_devices; curr_device++) {
    /* Shift 3 since CFGAR0 bitfield uses the last 5 bits (See Table 54 on p. 65) */
    config_packet->devices[curr_device].cfgA.gpio = (gpio_enable_pins[0] >> 3);
    config_packet->devices[curr_device].cfgB.gpio = (gpio_enable_pins[1] & 0x0F);

    uint16_t cfgr_pec = crc15_calculate((uint8_t *)&config_packet->devices[curr_device].cfgA, 6);
    config_packet->devices[curr_device].pecA = cfgr_pec >> 8 | cfgr_pec << 8; /* Swap endianness */

    cfgr_pec = crc15_calculate((uint8_t *)&config_packet->devices[curr_device].cfgB, 6);
    config_packet->devices[curr_device].pecB = cfgr_pec >> 8 | cfgr_pec << 8; /* Swap endianness */
  }

  size_t len = SIZEOF_ADBMS_AFE_WRITE_CONFIG_PACKET(settings->num_devices);
  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, (uint8_t *)config_packet, len, NULL, 0);
}

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

StatusCode adbms_afe_init(AdbmsAfeStorage *afe, const AdbmsAfeSettings *config) {
  /* Validate configuration settings based on device limitations */
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

  /* Initialize memory of AFE structure: reset values and copy configuration settings */
  memset(afe, 0, sizeof(*afe));
  memcpy(afe->settings, config, sizeof(AdbmsAfeSettings));

  /* Set up SPI communication based on provided settings */
  SpiSettings spi_config = afe->settings->spi_settings;

  spi_init(config->spi_port, &spi_config);

  /* Calculate offset for cell result array due to some cells being disabled */
  s_calc_offsets(afe);

  /* Initialize 15-bit CRC lookup table to optimize packet error code (PEC) calculation */
  crc15_init_table();

  /* Set same duty cycle for all cells in the AFE system */
  StatusCode status = adbms_afe_set_discharge_pwm_cycle(afe, ADBMS1818_PWMC_DC_100);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Read register failed");
    return status;
  }

  /*
   * Write configuration settings to AFE.
   * GPIO pins are configured with pull-down off: GPIO1 as analog input and GPIO3-5 for SPI
   */
  return adbms_afe_write_config(afe);
}

StatusCode adbms_afe_write_config(AdbmsAfeStorage *afe) {
  /* Double check, assuming only gpio2 stays on here */
  uint8_t gpio_bits[2];
  gpio_bits[0] = ADBMS1818_GPIO1_PD_OFF | ADBMS1818_GPIO3_PD_OFF | ADBMS1818_GPIO4_PD_OFF | ADBMS1818_GPIO5_PD_OFF;
  gpio_bits[1] = ADBMS1818_GPIO6_PD_OFF | ADBMS1818_GPIO7_PD_OFF | ADBMS1818_GPIO8_PD_OFF | ADBMS1818_GPIO9_PD_OFF;

  AdbmsAfeSettings *settings = afe->settings;

  for (uint8_t curr_device = 0; curr_device < settings->num_devices; curr_device++) {
    AdbmsAfeConfigRegisterAData *cfgrA = &afe->device_configs->devices[curr_device].cfgA;
    AdbmsAfeConfigRegisterBData *cfgrB = &afe->device_configs->devices[curr_device].cfgB;

    cfgrA->discharge_bitset = 0;
    cfgrA->discharge_timeout = ADBMS_AFE_DISCHARGE_TIMEOUT_30_S;

    cfgrA->adcopt = ((settings->adc_mode + 1) > 3);
    cfgrA->dten = true;
    cfgrA->refon = 0;

    cfgrA->undervoltage = 0;
    cfgrA->overvoltage = 0;

    cfgrB->discharge_bitset = 0;
    cfgrB->discharge_timer_monitor = 0;

    cfgrB->force_fail = 0;
    cfgrB->mute = 0;
    cfgrB->path_select = 0;

    cfgrB->reserved1 = 0;
    cfgrB->reserved2 = 0;
    cfgrB->reserved3 = 0;
    cfgrB->reserved4 = 0;
    /* If everything is zero could do:
       AdbmsAfeConfigRegisterBData cfgr B = {0} */
  }

  return s_write_config(afe, gpio_bits);
}

StatusCode adbms_afe_trigger_cell_conv(AdbmsAfeStorage *afe) {
  AdbmsAfeSettings *settings = afe->settings;

  /* See Table 39 (p. 61) for the MD[1:0] command bit description and values */
  uint8_t md_cmd_bits = (uint8_t)((settings->adc_mode) % (NUM_ADBMS_AFE_ADC_MODES / 2));

  /* ADVC Command Code (see Table 38 on p. 60) */
  uint16_t adcv = ADBMS1818_ADCV_RESERVED | ADBMS1818_ADCV_DISCHARGE_NOT_PERMITTED | ADBMS1818_CNVT_CELL_ALL | (md_cmd_bits << 7);

  uint8_t cmd[ADBMS1818_CMD_SIZE] = { 0 };
  s_build_cmd(adcv, cmd, ADBMS1818_CMD_SIZE);

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, ADBMS1818_CMD_SIZE, NULL, 0);
}

/* Start GPIOs ADC conversion and poll status for XXXXX */
StatusCode adbms_afe_trigger_aux_conv(AdbmsAfeStorage *afe, uint8_t thermistor) {
  AdbmsAfeSettings *settings = afe->settings;

  /* Left=shift by 3 since CFGR0 bitfield uses the last 5 bits (See Table 40 on p. 62) */
  uint8_t gpio_bits[2];
  gpio_bits[0] = (thermistor << 3) | ADBMS1818_GPIO4_PD_OFF;
  gpio_bits[1] = ADBMS1818_GPIO6_PD_OFF | ADBMS1818_GPIO7_PD_OFF | ADBMS1818_GPIO8_PD_OFF | ADBMS1818_GPIO9_PD_OFF;

  s_write_config(afe, gpio_bits);

  /* See Table 39 (p. 61) for the MD[1:0] command bit description and values */
  uint8_t md_cmd_bits = (uint8_t)((settings->adc_mode) % (NUM_ADBMS_AFE_ADC_MODES / 2));
  /* ADAX Command Code (see Table 38 on pg. 60) */
  uint16_t adax = ADBMS1818_ADAX_RESERVED | ADBMS1818_ADAX_GPIO4 | (md_cmd_bits << 7);

  uint8_t cmd[ADBMS1818_CMD_SIZE] = { 0 };
  s_build_cmd(adax, cmd, ADBMS1818_CMD_SIZE);

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, ADBMS1818_CMD_SIZE, NULL, 0);
}

StatusCode adbms_afe_read_cells(AdbmsAfeStorage *afe) {
  AdbmsAfeSettings *settings = afe->settings;

  for (AdbmsAfeVoltageRegister v_reg_group = ADBMS_AFE_VOLTAGE_REGISTER_A; v_reg_group < NUM_ADBMS_AFE_VOLTAGE_REGISTERS; ++v_reg_group) {
    /* Max devices macro used because value must be known at runtime */
    AdbmsAfeVoltageData devices_data[ADBMS_AFE_MAX_DEVICES] = { 0 };

    size_t len = sizeof(AdbmsAfeVoltageData) * settings->num_devices;
    StatusCode status = s_read_register(afe, (AdbmsAfeRegister)v_reg_group, (uint8_t *)devices_data, len);

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
      for (uint16_t cell = 0; cell < ADBMS1818_CELLS_IN_REG; ++cell) {
        uint16_t voltage = devices_data[device].reg.voltages[cell];

        /* Determine index of the battery cell */
        uint16_t device_cell = cell + (v_reg_group * ADBMS1818_CELLS_IN_REG);
        uint16_t index = device_cell + (device * ADBMS_AFE_MAX_CELLS_PER_DEVICE);

        /* Store voltage if cell status is enabled */
        if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
          afe->cell_voltages[afe->cell_result_lookup[index]] = voltage;
        }
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_read_aux(AdbmsAfeStorage *afe, uint8_t device_cell) {
  AdbmsAfeSettings *settings = afe->settings;
  AdbmsAfeAuxData devices_reg_data[ADBMS_AFE_MAX_DEVICES] = { 0 };

  size_t len = settings->num_devices * sizeof(AdbmsAfeAuxData);

  StatusCode status = s_read_register(afe, ADBMS_AFE_REGISTER_AUX_B, (uint8_t *)devices_reg_data, len);  // TODO: Double Check (if similar implementation)

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

    uint16_t index = device * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE + device_cell;
    afe->aux_voltages[index] = voltage;
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_toggle_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell, bool discharge) {
  if (cell >= ADBMS_AFE_MAX_CELLS) {
    LOG_DEBUG("Cell number is out of bounds");
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t device_index = cell / ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  uint16_t cell_indx_in_dev = cell % ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  if (cell_indx_in_dev < 12) {
    AdbmsAfeConfigRegisterAData *config_reg_data = &afe->device_configs->devices[device_index].cfgA;
    if (discharge) {
      config_reg_data->discharge_bitset |= (1 << cell_indx_in_dev);
    } else {
      config_reg_data->discharge_bitset &= ~(1 << cell_indx_in_dev);
    }
  } else {
    AdbmsAfeConfigRegisterBData *config_reg_data = &afe->device_configs->devices[device_index].cfgB;
    if (discharge) {
      config_reg_data->discharge_bitset |= (1 << cell_indx_in_dev);
    } else {
      config_reg_data->discharge_bitset &= ~(1 << cell_indx_in_dev);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_discharge_pwm_cycle(AdbmsAfeStorage *afe, uint8_t duty_cycle) {
  AdbmsAfeSettings *settings = afe->settings;

  /**
   * Initialize a buffer to store the PWM configuration command, while also accounting for
   * the number of registers to write to (6) for each AFE, which is not known at compile time.
   */
  uint8_t cmd_with_pwm[ADBMS1818_CMD_SIZE + (ADBMS1818_NUM_PWMR_REGS * ADBMS_AFE_MAX_DEVICES)] = { 0 };

  /* Same thing here but for the PWM/S control register group B */
  uint8_t cmd_with_psr[ADBMS1818_CMD_SIZE + (ADBMS1818_NUM_PSR_REGS * ADBMS_AFE_MAX_DEVICES)] = { 0 };

  /* Prepare command to be sent, disregarding the PWM configuration data */
  s_build_cmd(ADBMS1818_WRPWM_RESERVED, cmd_with_pwm, 4);
  s_build_cmd(ADBMS1818_WRPSB_RESERVED, cmd_with_psr, 4);

  /* Pack 4-bit duty cycle configuration for two PWM channels in an 8-bit PWM register */
  uint8_t packed_duty_cycle = (duty_cycle << 4) | duty_cycle;

  /* Set all 9 PWM registers to the same configuration for each AFE */
  for (uint8_t device = 0; device < settings->num_devices; device++) {
    /**
     * Since each AFE has 12 PWM channels for the PWMR group and each PWM register stores two 4-bit PWM values in a byte,
     * there are 6 registers to iterate through and 2 PWM channels are configured at a time.
     */
    for (int pwm_reg = 0; pwm_reg < ADBMS1818_NUM_PWMR_REGS; pwm_reg++) {
      /* Set both of the 2 PWM channels in the PWM register to the given configuration */
      cmd_with_pwm[ADBMS1818_CMD_SIZE + (device * ADBMS1818_NUM_PWMR_REGS) + pwm_reg] = packed_duty_cycle;
    }

    /* Same thing here, but there are 3 registers to itterate through per device since the PSR group has 3 PWM register groups */
    for (int pwm_reg = 0; pwm_reg < ADBMS1818_NUM_PWM_REGS_IN_PSR; pwm_reg++) {
      cmd_with_psr[ADBMS1818_CMD_SIZE + (device * ADBMS1818_NUM_PSR_REGS) + pwm_reg] = packed_duty_cycle;
    }
    /* SCTL regs are zeroed out when cmd_with_psr is initialized */
  }

  size_t cmd_with_pwm_len = ADBMS1818_CMD_SIZE + (ADBMS1818_NUM_PWMR_REGS * settings->num_devices);
  size_t cmd_with_psr_len = ADBMS1818_CMD_SIZE + (ADBMS1818_NUM_PSR_REGS * settings->num_devices);

  /* Wake up the devices if they are idle before sending the command */
  s_wakeup_idle(afe);

  /* Send PWM configuration command to all AFEs */
  StatusCode ret = spi_exchange(settings->spi_port, cmd_with_pwm, cmd_with_pwm_len, NULL, 0);
  if (ret != STATUS_CODE_OK) {
    return ret;
  }
  return (spi_exchange(settings->spi_port, cmd_with_psr, cmd_with_psr_len, NULL, 0));
}
