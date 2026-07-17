/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for rc_current_sense
 *
 * @details Closes the pack relays (POS, SOLAR, NEG) then continuously samples the ADS122C14
 *          current-sense ADC, reporting pack voltage (HV divider) and pack current (shunt).
 *          Mirrors the sampling/conversion path in projects/rear_controller/src/current_sense.c
 *          so the hardware can be bench-verified without the full state manager / CAN stack.
 *
 * @date   2026-07-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "current_ads122c14irter.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "rear_controller_hw_defs.h"

/* ADS122 config mirrored from current_sense.c (rev 3 / IS_USING_CURRENT_SENSE_REV_3) */
#define CSENSE_FSR 5                        /**< Full-scale range: Vref(2.5V) / Gain(0.5) */
#define CSENSE_MUX_SHUNT 0x67               /**< AIN6 (+) / AIN7 (-): shunt current path */
#define CSENSE_MUX_HV 0x01                  /**< AIN0 (+) / AIN1 (-): HV divider path */
#define CSENSE_SHUNT_RESISTANCE_OHM 0.0005f /**< 0.5 mOhm current-sense shunt */
#define CSENSE_R6_OHM 1000000.0f            /**< HV divider top resistor */
#define CSENSE_R7_OHM 20000.0f              /**< HV divider bottom resistor */

#define CSENSE_CONVERSION_FULL_SCALE (1 << 23) /**< 24-bit signed ADC full-scale */
#define CSENSE_DATA_READY_MASK 0x01U
#define CSENSE_DRDY_POLL_TIMEOUT_MS 100U
#define CSENSE_SAMPLE_PERIOD_MS 500U
#define CSENSE_CLOSE_RELAYS_DELAY_MS 250U /**< Settle time after driving a relay enable (mirrors relays.c) */

static ADS122Storage s_ads122_storage;

static I2CSettings s_i2c_settings = {
  .speed = I2C_SPEED_FAST,
  .sda = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SDA_GPIO,
  .scl = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SCL_GPIO,
};

/* Relay enable pins - closing a relay drives its enable high */
static GpioAddress s_pos_relay_en = GPIO_REAR_CONTROLLER_POS_RELAY_ENABLE;
static GpioAddress s_solar_relay_en = GPIO_REAR_CONTROLLER_SOLAR_RELAY_ENABLE;
static GpioAddress s_neg_relay_en = GPIO_REAR_CONTROLLER_NEG_RELAY_ENABLE;

static uint8_t s_register_map[] = { ADS122_REG_DEVICE_CFG_DEFAULT,
                                    ADS122_REG_DATA_RATE_CFG_DEFAULT,
                                    (ADS122_REG_MUX_CFG_DEFAULT | 0x01),       /* reads voltage first */
                                    ADS122_REG_GAIN_CFG_DEFAULT,               /* Gain = 0.5 */
                                    (ADS122_REG_REFERENCE_CFG_DEFAULT | 0x04), /* Vref = 2.5 V -> +-5 V range */
                                    (ADS122_REG_DIGITAL_CFG_DEFAULT | 0x10),
                                    ADS122_REG_GPIO_CFG_DEFAULT,
                                    ADS122_REG_GPIO_DATA_OUTPUT_DEFAULT,
                                    ADS122_REG_IDAC_MAG_CFG_DEFAULT,
                                    ADS122_REG_IDAC_MUX_CFG_DEFAULT,
                                    ADS122_REG_REG_MAP_CRC_DEFAULT };

/* Print a float as a signed fixed-point value with 3 decimals (avoids soft-float printf) */
static void s_log_fixed_3dp(const char *label, const char *unit, float value) {
  int32_t milli = (int32_t)(value * 1000.0f);
  uint32_t whole = (uint32_t)(milli < 0 ? -(milli / 1000) : (milli / 1000));
  uint32_t frac = (uint32_t)(milli < 0 ? -(milli % 1000) : (milli % 1000));
  LOG_DEBUG("%s: %s%u.%03u %s\r\n", label, milli < 0 ? "-" : "", (unsigned)whole, (unsigned)frac, unit);
}

/* Blocking single-shot read on the given MUX; returns the raw differential voltage in volts */
static StatusCode s_read_channel(uint8_t mux_config, float *voltage_V) {
  status_ok_or_return(ads122_change_MUX(&s_ads122_storage, mux_config));
  status_ok_or_return(ads122_start_conversion(&s_ads122_storage));

  uint8_t raw[5U] = { 0U };
  for (uint32_t elapsed = 0U; elapsed < CSENSE_DRDY_POLL_TIMEOUT_MS; elapsed += 2U) {
    status_ok_or_return(ads122_get_conversion_data(&s_ads122_storage, raw));
    if (raw[0] & CSENSE_DATA_READY_MASK) {
      uint32_t conversion_data = ((uint32_t)raw[2] << 16) | ((uint32_t)raw[3] << 8) | ((uint32_t)raw[4]);
      int32_t conversion_signed = (int32_t)(conversion_data << 8) >> 8;
      *voltage_V = (float)(conversion_signed * CSENSE_FSR) / (float)CSENSE_CONVERSION_FULL_SCALE;
      return STATUS_CODE_OK;
    }
    delay_ms(2U);
  }

  return STATUS_CODE_TIMEOUT;
}

static void s_close_pack_relays(void) {
  gpio_init_pin(&s_pos_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&s_solar_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&s_neg_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  gpio_set_state(&s_pos_relay_en, GPIO_STATE_HIGH);
  delay_ms(CSENSE_CLOSE_RELAYS_DELAY_MS);
  gpio_set_state(&s_solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(CSENSE_CLOSE_RELAYS_DELAY_MS);
  gpio_set_state(&s_neg_relay_en, GPIO_STATE_HIGH);
  delay_ms(CSENSE_CLOSE_RELAYS_DELAY_MS);

  LOG_DEBUG("Relays closed: POS, SOLAR, NEG\r\n");
}

TASK(rc_current_sense_smoke, TASK_STACK_1024) {
  i2c_init(REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, &s_i2c_settings);

  if (ads122_init(&s_ads122_storage, REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, REAR_CONTOLLER_CURRENT_SENSE_ADC122_I2C_ADDR, s_register_map, &s_i2c_settings) != STATUS_CODE_OK) {
    LOG_DEBUG("ADS122 init FAILED\r\n");
  }

  s_close_pack_relays();

  while (true) {
    LOG_DEBUG("---- RC CURRENT SENSE SMOKE ----\r\n");

    float shunt_voltage_V = 0.0f;
    if (s_read_channel(CSENSE_MUX_SHUNT, &shunt_voltage_V) == STATUS_CODE_OK) {
      float current_A = shunt_voltage_V / CSENSE_SHUNT_RESISTANCE_OHM;
      s_log_fixed_3dp("PACK_CURRENT", "A", current_A);
    } else {
      LOG_DEBUG("ERROR reading shunt (current)\r\n");
    }

    float hv_voltage_V = 0.0f;
    if (s_read_channel(CSENSE_MUX_HV, &hv_voltage_V) == STATUS_CODE_OK) {
      float pack_voltage_V = hv_voltage_V * (CSENSE_R6_OHM + CSENSE_R7_OHM) / CSENSE_R7_OHM;
      s_log_fixed_3dp("PACK_VOLTAGE", "V", pack_voltage_V);
    } else {
      LOG_DEBUG("ERROR reading HV divider (voltage)\r\n");
    }

    delay_ms(CSENSE_SAMPLE_PERIOD_MS);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();
  LOG_DEBUG("Starting rc_current_sense smoke test...\r\n");

  tasks_init_task(rc_current_sense_smoke, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
