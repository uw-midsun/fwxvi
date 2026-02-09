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
  [ADBMS_AFE_REGISTER_CELL_VOLTAGE_E] = ADBMS1818_RDCVE_RESERVED,  [ADBMS_AFE_REGISTER_CELL_VOLTAGE_F] = ADBMS1818_RDCVF_RESERVED,
  [ADBMS_AFE_REGISTER_AUX_A] = ADBMS1818_RDAUXA_RESERVED,     [ADBMS_AFE_REGISTER_AUX_B] = ADBMS1818_RDAUXB_RESERVED,         [ADBMS_AFE_REGISTER_AUX_C] = ADBMS1818_RDAUXC_RESERVED,
  [ADBMS_AFE_REGISTER_AUX_D] = ADBMS1818_RDAUXD_RESERVED,
  [ADBMS_AFE_REGISTER_STATUS_A] = ADBMS1818_RDSTATA_RESERVED, [ADBMS_AFE_REGISTER_STATUS_B] = ADBMS1818_RDSTATB_RESERVED,     [ADBMS_AFE_REGISTER_READ_COMM] = ADBMS1818_RDCOMM_RESERVED,
  [ADBMS_AFE_REGISTER_START_COMM] = ADBMS1818_STCOMM_RESERVED
};

/* p. 56-57 - Daisy chain wakeup method 2 - pair of long -1, +1 for each device */
static void s_wakeup_idle(AdbmsAfeStorage *afe) {
  AdbmsAfeSettings *settings = afe->settings;

  for (size_t i = 0; i < settings->num_devices; i++) {
    gpio_set_state(&settings->spi_settings->cs, GPIO_STATE_LOW);
    gpio_set_state(&settings->spi_settings->cs, GPIO_STATE_HIGH);
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
  uint8_t cmd[ADBMS1818_CMD_SIZE] = { 0 };
  s_build_cmd(reg_cmd, cmd, ADBMS1818_CMD_SIZE);

  s_wakeup_idle(afe);

  /* For isoSPI: send command, then clock in response data */
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

static StatusCode s_write_cfga(AdbmsAfeStorage *afe) {
  if (!afe) return STATUS_CODE_INVALID_ARGS;
  AdbmsAfeSettings *settings = afe->settings;

  /* Calculate PEC for each device's config data */
  for (uint8_t i = 0; i < settings->num_devices; i++) {
    uint16_t pec = crc15_calculate((uint8_t *)&afe->config_a[i].cfg, sizeof(AdbmsAfeConfigRegisterAData));
    afe->config_a[i].pec = (uint16_t)((pec >> 8) | (pec << 8));
  }

  /* Build command + data buffer: CMD(4) + DATA(6*num_devices) */
  size_t total_len = ADBMS1818_CMD_SIZE + (settings->num_devices * sizeof(AdbmsAfeWriteConfigAPacket));
  uint8_t tx_buffer[ADBMS1818_CMD_SIZE + (ADBMS_AFE_MAX_DEVICES * sizeof(AdbmsAfeWriteConfigAPacket))] = { 0 };

  uint8_t *raw = (uint8_t*)&afe->config_a[0].cfg;
for(int i = 0; i < 6; i++) {
  LOG_DEBUG("  Byte[%d]: 0x%02X\r\n", i, raw[i]);
  delay_ms(10);
}

  /* Add command to buffer */
  s_build_cmd(ADBMS1818_WRCFGA_RESERVED, tx_buffer, ADBMS1818_CMD_SIZE);

  /* Add data packets to buffer */
  memcpy(&tx_buffer[ADBMS1818_CMD_SIZE], afe->config_a, settings->num_devices * sizeof(AdbmsAfeWriteConfigAPacket));

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, tx_buffer, total_len, NULL, 0);
}

static StatusCode s_write_cfgb(AdbmsAfeStorage *afe) {
  if (!afe) return STATUS_CODE_INVALID_ARGS;
  AdbmsAfeSettings *settings = afe->settings;

  /* Calculate PEC for each device's config data */
  for (uint8_t i = 0; i < settings->num_devices; i++) {
    uint16_t pec = crc15_calculate((uint8_t *)&afe->config_b[i].cfg, sizeof(AdbmsAfeConfigRegisterBData));
    afe->config_b[i].pec = (uint16_t)((pec >> 8) | (pec << 8));
  }

  /* Build command + data buffer */
  size_t total_len = ADBMS1818_CMD_SIZE + (settings->num_devices * sizeof(AdbmsAfeWriteConfigBPacket));
  uint8_t tx_buffer[ADBMS1818_CMD_SIZE + (ADBMS_AFE_MAX_DEVICES * sizeof(AdbmsAfeWriteConfigBPacket))] = { 0 };

  /* Add command to buffer */
  s_build_cmd(ADBMS1818_WRCFGB_RESERVED, tx_buffer, ADBMS1818_CMD_SIZE);

  /* Add data packets to buffer */
  memcpy(&tx_buffer[ADBMS1818_CMD_SIZE], afe->config_b, settings->num_devices * sizeof(AdbmsAfeWriteConfigBPacket));

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, tx_buffer, total_len, NULL, 0);
}

static StatusCode s_write_config(AdbmsAfeStorage *afe, const uint8_t *gpio_enable_pins) {
  if (!afe) return STATUS_CODE_INVALID_ARGS;

  if (gpio_enable_pins) {
    for (uint8_t i = 0; i < afe->settings->num_devices; i++) {
      afe->config_a[i].cfg.gpio = (gpio_enable_pins[0] >> 3U);
      afe->config_b[i].cfg.gpio = gpio_enable_pins[1];
    }
  }

  StatusCode status = s_write_cfga(afe);
  if (status != STATUS_CODE_OK) return status;

  delay_ms(50);

  status = s_write_cfgb(afe);
  if (status != STATUS_CODE_OK) return status;

  delay_ms(50);

  return STATUS_CODE_OK;
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
    size_t device_offset = device * ADBMS_AFE_MAX_CELLS_PER_DEVICE;

    for (size_t device_cell = 0; device_cell < ADBMS_AFE_MAX_CELLS_PER_DEVICE; device_cell++) {
      size_t raw_cell_index = device_offset + device_cell;

      afe->discharge_cell_lookup[enabled_cell_index] = raw_cell_index;
      afe->cell_result_lookup[raw_cell_index] = enabled_cell_index++;
    }
  }
}

StatusCode adbms_afe_init(AdbmsAfeStorage *afe, const AdbmsAfeSettings *config) {
  if (afe == NULL || config == NULL || config->spi_settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

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

  memset(afe, 0, sizeof(*afe));
  afe->settings = config;

  spi_init(config->spi_port, config->spi_settings);

  s_calc_offsets(afe);

  crc15_init_table();

  for (uint8_t i = 0; i < config->num_devices; i++) {
    AdbmsAfeConfigRegisterAData *cfgrA = &afe->config_a[i].cfg;
    AdbmsAfeConfigRegisterBData *cfgrB = &afe->config_b[i].cfg;

    memset(cfgrA, 0, sizeof(*cfgrA));
    memset(cfgrB, 0, sizeof(*cfgrB));

    cfgrA->discharge_timeout = ADBMS_AFE_DISCHARGE_TIMEOUT_1_MIN;
    cfgrA->adcopt = ((config->adc_mode + 1) > 3);
    cfgrA->dten = true;
    cfgrA->refon = true;
  }

  StatusCode status = adbms_afe_set_discharge_pwm_cycle(afe, ADBMS1818_PWMC_DC_100);

  if (status != STATUS_CODE_OK) {
    return status;
  }

  return adbms_afe_write_config(afe);
}

StatusCode adbms_afe_write_config(AdbmsAfeStorage *afe) {
  if (!afe) return STATUS_CODE_INVALID_ARGS;

  uint8_t gpio_bits[2];

  gpio_bits[0] = ADBMS1818_GPIO1_PD_OFF | ADBMS1818_GPIO2_PD_OFF |
                 ADBMS1818_GPIO3_PD_OFF | ADBMS1818_GPIO4_PD_OFF |
                 ADBMS1818_GPIO5_PD_OFF;

  gpio_bits[1] = ADBMS1818_GPIO6_PD_OFF | ADBMS1818_GPIO7_PD_OFF |
                 ADBMS1818_GPIO8_PD_OFF | ADBMS1818_GPIO9_PD_OFF;

  return s_write_config(afe, gpio_bits);
}

StatusCode adbms_afe_trigger_cell_conv(AdbmsAfeStorage *afe) {
  AdbmsAfeSettings *settings = afe->settings;

  uint8_t md_cmd_bits = (uint8_t)((settings->adc_mode) % (NUM_ADBMS_AFE_ADC_MODES / 2));

  uint16_t adcv = ADBMS1818_ADCV_RESERVED | ADBMS1818_ADCV_DISCHARGE_NOT_PERMITTED | ADBMS1818_CNVT_CELL_ALL | (md_cmd_bits << 7);

  uint8_t cmd[ADBMS1818_CMD_SIZE] = { 0 };
  s_build_cmd(adcv, cmd, ADBMS1818_CMD_SIZE);

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, ADBMS1818_CMD_SIZE, NULL, 0);
}

StatusCode adbms_afe_trigger_thermistor_conv(AdbmsAfeStorage *afe, uint8_t device_num, uint8_t index) {
  if (!afe || !afe->settings) {
    return STATUS_CODE_INVALID_ARGS;
  }
  AdbmsAfeSettings *settings = afe->settings;

  if (device_num >= settings->num_devices || index >= 16) {
    return STATUS_CODE_INVALID_ARGS;
  }

  bool use_gpio5 = (index >= 8);
  uint8_t mux_index = index % 8;

  uint8_t gpio_bits[2] = { 0 };
  gpio_bits[0] = (mux_index << 3) | (use_gpio5 ? ADBMS1818_GPIO5_PD_OFF : ADBMS1818_GPIO4_PD_OFF);
  gpio_bits[1] = ADBMS1818_GPIO6_PD_OFF | ADBMS1818_GPIO7_PD_OFF | ADBMS1818_GPIO8_PD_OFF | ADBMS1818_GPIO9_PD_OFF;

  StatusCode status = s_write_config(afe, gpio_bits);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint8_t md_cmd_bits = (uint8_t)((settings->adc_mode) % (NUM_ADBMS_AFE_ADC_MODES / 2));
  uint16_t adax = ADBMS1818_ADAX_RESERVED | ADBMS1818_ADAX_GPIO_ALL | (md_cmd_bits << 7);

  uint8_t cmd[ADBMS1818_CMD_SIZE] = { 0 };
  status = s_build_cmd(adax, cmd, ADBMS1818_CMD_SIZE);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, ADBMS1818_CMD_SIZE, NULL, 0);
}

StatusCode adbms_afe_trigger_board_temp_conv(AdbmsAfeStorage *afe, uint8_t device_num) {
  if (!afe || !afe->settings) {
    return STATUS_CODE_INVALID_ARGS;
  }

  AdbmsAfeSettings *settings = afe->settings;

  if (device_num >= settings->num_devices) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t gpio_bits[2] = { 0 };
  gpio_bits[0] = ADBMS1818_GPIO1_PD_OFF | ADBMS1818_GPIO2_PD_OFF | ADBMS1818_GPIO3_PD_OFF | ADBMS1818_GPIO4_PD_OFF | ADBMS1818_GPIO5_PD_OFF;
  gpio_bits[1] = ADBMS1818_GPIO6_PD_OFF | ADBMS1818_GPIO7_PD_OFF | ADBMS1818_GPIO8_PD_OFF | ADBMS1818_GPIO9_PD_OFF;

  StatusCode status = s_write_config(afe, gpio_bits);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint8_t md_cmd_bits = (uint8_t)((settings->adc_mode) % (NUM_ADBMS_AFE_ADC_MODES / 2));
  uint16_t adax = ADBMS1818_ADAX_RESERVED | ADBMS1818_ADAX_GPIO_ALL | (md_cmd_bits << 7);

  uint8_t cmd[ADBMS1818_CMD_SIZE] = { 0 };
  status = s_build_cmd(adax, cmd, ADBMS1818_CMD_SIZE);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  s_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, ADBMS1818_CMD_SIZE, NULL, 0);
}

StatusCode adbms_afe_read_cells(AdbmsAfeStorage *afe) {
  AdbmsAfeSettings *settings = afe->settings;

  for (AdbmsAfeVoltageRegister v_reg_group = ADBMS_AFE_VOLTAGE_REGISTER_A; v_reg_group < NUM_ADBMS_AFE_VOLTAGE_REGISTERS; ++v_reg_group) {
    AdbmsAfeVoltageData devices_data[ADBMS_AFE_MAX_DEVICES] = { 0 };

    size_t len = sizeof(AdbmsAfeVoltageData) * settings->num_devices;
    StatusCode status = s_read_register(afe, (AdbmsAfeRegister)v_reg_group, (uint8_t *)devices_data, len);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Read register failed");
      return status;
    }

    for (uint8_t device = 0; device < settings->num_devices; ++device) {
      uint16_t received_pec = (devices_data[device].pec >> 8) | (devices_data[device].pec << 8);
      uint16_t data_pec = crc15_calculate((uint8_t *)&devices_data[device], 6);

      if (data_pec != received_pec) {
        LOG_DEBUG("Communication Failed with device: %d, DATA_PEC: %d, RECEIVED_PEC: %d\n\r", device, data_pec, received_pec);
        LOG_DEBUG("Voltage: %d %d %d\n\r", devices_data[device].reg.voltages[0], devices_data[device].reg.voltages[1], devices_data[device].reg.voltages[2]);
        return STATUS_CODE_INTERNAL_ERROR;
      }

      for (uint16_t cell = 0; cell < ADBMS1818_CELLS_IN_REG; ++cell) {
        uint16_t voltage = devices_data[device].reg.voltages[cell];

        uint16_t device_cell = cell + ((v_reg_group - ADBMS_AFE_VOLTAGE_REGISTER_A) * ADBMS1818_CELLS_IN_REG);
        uint16_t index = device_cell + (device * ADBMS_AFE_MAX_CELLS_PER_DEVICE);

        afe->cell_voltages[afe->cell_result_lookup[index]] = voltage;
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode adbms_afe_read_thermistor(AdbmsAfeStorage *afe, uint8_t device_num, uint8_t index) {
  if (!afe || !afe->settings) {
    return STATUS_CODE_INVALID_ARGS;
  }

  AdbmsAfeSettings *settings = afe->settings;

  if (device_num >= settings->num_devices || index >= 16) {
    return STATUS_CODE_INVALID_ARGS;
  }

  bool use_gpio5 = (index >= 8);

  AdbmsAfeAuxData devices_reg_data[ADBMS_AFE_MAX_DEVICES] = { 0 };
  size_t len = sizeof(AdbmsAfeAuxData) * settings->num_devices;

  StatusCode status = s_read_register(afe, ADBMS_AFE_REGISTER_AUX_B, (uint8_t *)devices_reg_data, len);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint16_t received_pec = (devices_reg_data[device_num].pec >> 8) | (devices_reg_data[device_num].pec << 8);
  uint16_t data_pec = crc15_calculate((uint8_t *)&devices_reg_data[device_num], 6);
  if (received_pec != data_pec) {
    LOG_DEBUG("PEC mismatch on Thermistor read: dev=%u", device_num);
    LOG_DEBUG("RECEIVED_PEC: %d\n\r", received_pec);
    LOG_DEBUG("DATA_PEC: %d\n\r", data_pec);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  uint16_t raw = devices_reg_data[device_num].reg.voltages[use_gpio5 ? 1 : 0];

  uint16_t global_index = device_num * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + index;
  if (global_index < settings->num_thermistors) {
    afe->thermistor_voltages[global_index] = raw;
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_read_board_temp(AdbmsAfeStorage *afe, uint8_t device_num) {
  if (!afe || !afe->settings) {
    return STATUS_CODE_INVALID_ARGS;
  }

  AdbmsAfeSettings *settings = afe->settings;

  if (device_num >= settings->num_devices) {
    return STATUS_CODE_INVALID_ARGS;
  }

  AdbmsAfeAuxData devices_reg_data[ADBMS_AFE_MAX_DEVICES] = { 0 };
  size_t len = sizeof(AdbmsAfeAuxData) * settings->num_devices;

  StatusCode status = s_read_register(afe, ADBMS_AFE_REGISTER_AUX_C, (uint8_t *)devices_reg_data, len);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint16_t received_pec = (devices_reg_data[device_num].pec >> 8) | (devices_reg_data[device_num].pec << 8);
  uint16_t data_pec = crc15_calculate((uint8_t *)&devices_reg_data[device_num], 6);

  if (received_pec != data_pec) {
    LOG_DEBUG("PEC mismatch on AUXC read (board temp dev=%u)", device_num);
    LOG_DEBUG("RECEIVED_PEC: %d\n\r", received_pec);
    LOG_DEBUG("DATA_PEC: %d\n\r", data_pec);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  uint16_t raw = devices_reg_data[device_num].reg.voltages[0];
  afe->board_thermistor_voltages[device_num] = raw;

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_toggle_cell_discharge(AdbmsAfeStorage *afe, uint16_t cell, bool discharge) {
  if (cell >= ADBMS_AFE_MAX_CELLS || cell <= 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t device_index = cell / ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  uint16_t cell_indx_in_dev = (cell % ADBMS_AFE_MAX_CELLS_PER_DEVICE) + 1;

  if (cell_indx_in_dev <= 12) {
    /* Cells 1-12 belong to CFGRA */
    AdbmsAfeConfigRegisterAData *cfgA = &afe->config_a[device_index].cfg;
    LOG_DEBUG("Device_index: %u, cell_indx: %u\r\n", device_index, cell_indx_in_dev);
    delay_ms(10);
    uint8_t bit_index = cell_indx_in_dev - 1U;
    if (discharge) {
      cfgA->discharge_bitset |= (1U << bit_index);
    } else {
      cfgA->discharge_bitset &= ~(1U << bit_index);
    }
  } else {
    /* Cells 13-18 belong to CFGRB */
    AdbmsAfeConfigRegisterBData *cfgB = &afe->config_b[device_index].cfg;

    uint8_t bit_index = cell_indx_in_dev - 13U;
    if (discharge) {
      cfgB->discharge_bitset |= (1U << bit_index);
    } else {
      cfgB->discharge_bitset &= ~(1U << bit_index);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode adbms_afe_set_discharge_pwm_cycle(AdbmsAfeStorage *afe, uint8_t duty_cycle) {
  AdbmsAfeSettings *settings = afe->settings;

  /* 0–100% -> 0–15 (PWMC code), rounded */
  uint8_t pwmc_value = (duty_cycle >= 100) ? 15 : (uint8_t)((duty_cycle * 15u + 50u) / 100u);
  pwmc_value &= 0x0F;

  uint8_t packed_duty_cycle = (uint8_t)((pwmc_value << 4) | pwmc_value);

  /* Build WRPWM command with data + PEC (6 bytes data + 2 bytes PEC per device) */
  size_t pwm_packet_len = ADBMS1818_NUM_PWMR_REGS + 2;
  size_t pwm_data_len = pwm_packet_len * settings->num_devices;
  size_t pwm_total_len = ADBMS1818_CMD_SIZE + pwm_data_len;
  uint8_t cmd_with_pwm[ADBMS1818_CMD_SIZE + ((ADBMS1818_NUM_PWMR_REGS + 2) * ADBMS_AFE_MAX_DEVICES)] = { 0 };

  s_build_cmd(ADBMS1818_WRPWM_RESERVED, cmd_with_pwm, ADBMS1818_CMD_SIZE);

  for (uint8_t device = 0; device < settings->num_devices; device++) {
    uint8_t *device_data = &cmd_with_pwm[ADBMS1818_CMD_SIZE + (device * pwm_packet_len)];
    for (int pwm_reg = 0; pwm_reg < ADBMS1818_NUM_PWMR_REGS; pwm_reg++) {
      device_data[pwm_reg] = packed_duty_cycle;
    }
    /* Calculate and append PEC for this device's data */
    uint16_t pec = crc15_calculate(device_data, ADBMS1818_NUM_PWMR_REGS);
    device_data[ADBMS1818_NUM_PWMR_REGS] = (uint8_t)(pec >> 8);
    device_data[ADBMS1818_NUM_PWMR_REGS + 1] = (uint8_t)(pec & 0xFF);
  }

  s_wakeup_idle(afe);

  StatusCode ret = spi_exchange(settings->spi_port, cmd_with_pwm, pwm_total_len, NULL, 0);
  if (ret != STATUS_CODE_OK) {
    return ret;
  }

  /* Build WRPSB command with data + PEC (6 bytes data + 2 bytes PEC per device) */
  size_t psr_packet_len = ADBMS1818_NUM_PSR_REGS + 2;
  size_t psr_data_len = psr_packet_len * settings->num_devices;
  size_t psr_total_len = ADBMS1818_CMD_SIZE + psr_data_len;
  uint8_t cmd_with_psr[ADBMS1818_CMD_SIZE + ((ADBMS1818_NUM_PSR_REGS + 2) * ADBMS_AFE_MAX_DEVICES)] = { 0 };

  s_build_cmd(ADBMS1818_WRPSB_RESERVED, cmd_with_psr, ADBMS1818_CMD_SIZE);

  for (uint8_t device = 0; device < settings->num_devices; device++) {
    uint8_t *device_data = &cmd_with_psr[ADBMS1818_CMD_SIZE + (device * psr_packet_len)];
    for (int pwm_reg = 0; pwm_reg < ADBMS1818_NUM_PWM_REGS_IN_PSR; pwm_reg++) {
      device_data[pwm_reg] = packed_duty_cycle;
    }
    /* Calculate and append PEC for this device's data */
    uint16_t pec = crc15_calculate(device_data, ADBMS1818_NUM_PSR_REGS);
    device_data[ADBMS1818_NUM_PSR_REGS] = (uint8_t)(pec >> 8);
    device_data[ADBMS1818_NUM_PSR_REGS + 1] = (uint8_t)(pec & 0xFF);
  }

  s_wakeup_idle(afe);

  return spi_exchange(settings->spi_port, cmd_with_psr, psr_total_len, NULL, 0);
}
