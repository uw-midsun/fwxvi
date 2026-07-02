/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for rear_controller_killswitch
 *
 * @brief  Tests GPIO initialization and interrupt setup for killswitch without calling
 *         killswitch.h directly. This allows testing the killswitch API pattern and behavior.
 *
 * @date   2026-01-07
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "log.h"
#include "mcu.h"
#include "notify.h"
#include "status.h"
#include "tasks.h"
#include "current_ads122c14irter.h"

/* FSR = Vref / Gain -> Vref = 2.5, Gain = 0.5*/
#define csense_FSR 5
#define csense_AIN6_AIN7_MUX_CFG 0x67 /*shunt inputs*/
#define csense_AIN0_AIN1_MUX_CDF 0x01 /*HV_BUS and BAT_GND*/
#define csense_R6_ohm 1000000 /*1M ohm resistor*/
#define csense_R7_ohm 20000 /*20k ohm resistor*/

static ADS122Storage ads122_storage = {};

static uint8_t register_map[] = {
  ADS122_REG_DEVICE_CFG_DEFAULT,
  ADS122_REG_DATA_RATE_CFG_DEFAULT,
  (ADS122_REG_MUX_CFG_DEFAULT | csense_AIN0_AIN1_MUX_CDF), //reads voltage first
  ADS122_REG_GAIN_CFG_DEFAULT, // Gain is 0.5
  (ADS122_REG_REFERENCE_CFG_DEFAULT | 0x04), //Vref = 2.5 V -> max range is +- 5 V, clock speed is 256 kHz
  (ADS122_REG_DIGITAL_CFG_DEFAULT | 0x10),  /* TODO: CHANGE BACK*/
  ADS122_REG_GPIO_CFG_DEFAULT,
  ADS122_REG_GPIO_DATA_OUTPUT_DEFAULT,
  ADS122_REG_IDAC_MAG_CFG_DEFAULT,
  ADS122_REG_IDAC_MUX_CFG_DEFAULT,
  ADS122_REG_REG_MAP_CRC_DEFAULT
};

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  interrupt_init();
  tasks_init();
  log_init();

  tasks_start();

  const I2CSettings i2c_settings = {
    .sda = {.port = GPIO_PORT_B, .pin = 11U},
    .scl = {.port = GPIO_PORT_B, .pin = 10U },
  };

  StatusCode init_status = ads122_init(&ads122_storage, I2C_PORT_2, 64, register_map, &i2c_settings);

  LOG_DEBUG("exiting main?\r\n");
  return 0;
}
