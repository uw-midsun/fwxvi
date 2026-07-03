/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for rear_controller_current_sense
 *
 * @brief  Tests i2c init, read, write and conversion.
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
#define LOG_DB_DELAY 10U

static ADS122Storage ads122_storage = { 0 };

typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;
  I2CSettings i2c_settings;
} ADS122Storage;

  const I2CSettings i2c_settings = {
    .sda = {.port = GPIO_PORT_B, .pin = 11U},
    .scl = {.port = GPIO_PORT_B, .pin = 10U },
  };

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


TASK(current_sense_run_cycle, TASK_STACK_1024) {
  LOG_DEBUG("Initializing current sense...\r\n");
  delay_ms(LOG_DB_DELAY);

  StatusCode init_status = ads122_init(&ads122_storage, I2C_PORT_2, 64, register_map, &i2c_settings);
  if (init_status == STATUS_CODE_OK) {
    LOG_DEBUG("Current sense initialized\r\n");
  } else {
    LOG_DEBUG("Current Sense cannot be initialized\r\n");
  }
  

  while (true) {

    uint8_t tx_data[2];
    
    //Set MUX config here -> change tx_data [1]
    tx_data [0] = ads122_create_command(ADS122_REG_MUX_CFG, ADS122_WRITE_COMMAND);
    tx_data [1] = csense_AIN0_AIN1_MUX_CDF; //VOLTAGE
    // tx_data [1] = csense_AIN6_AIN7_MUX_CFG; // CURRENT

    StatusCode status_write = i2c_write(I2C_PORT_2, 64, tx_data, 2U);
    if (status_write == STATUS_CODE_OK) {
      LOG_DEBUG("Current sense writing ADS122_REG_MUX_CFG (change mux config)\r\n");
    } else {
      LOG_DEBUG("Current sense not reading ADS122_REG_MUX_CFG (change mux config)\r\n");
    }
  
    //Start converison
    uint8_t reg_address = ads122_create_command(ADS122_REG_CONVERSION_CTRL, ADS122_READ_COMMAND);
    uint8_t conversion_ctrl = 0x00;

    StatusCode status_read = (i2c_read_mem(I2C_PORT_2, 64, reg_address, &conversion_ctrl, 1U));
    if (status_read == STATUS_CODE_OK) {
      LOG_DEBUG("Current sense reading ADS122_REG_CONVERSION_CTRL\r\n");
    } else {
      LOG_DEBUG("Current sense not reading ADS122_REG_CONVERSION_CTRL\r\n");
    }

    tx_data[0] = ads122_create_command(ADS122_REG_CONVERSION_CTRL, ADS122_WRITE_COMMAND);
    conversion_ctrl |= (1 << 1);
    tx_data [1] = conversion_ctrl;

    status_write = i2c_write(I2C_PORT_2, 64, tx_data, 2U);
    if (status_write == STATUS_CODE_OK) {
      LOG_DEBUG("Current sense writing ADS122_REG_CONVERSION_CTRL\r\n");
    } else {
      LOG_DEBUG("Current sense not writing ADS122_REG_CONVERSION_CTRL\r\n");
    }
    
    uint8_t raw_data[5];

    StatusCode status_conversion  = i2c_read_mem(I2C_PORT_2, 64, ADS122_READ_CONVERSION_COMMAND, raw_data, 5U);
    if (status_conversion == STATUS_CODE_OK) {
    LOG_DEBUG("Current sense converting\r\n");
    } else {
    LOG_DEBUG("Current Sense is not converting\r\n");
    }

    bool data_ready = raw_data[0] & 0x01;
    
    if(data_ready){
      bool negative = raw_data[2] & 0x80; // change to right values

      uint32_t cs_conversion_data = ((uint32_t)raw_data[2] << 16) | ((uint32_t)raw_data[3] << 8) | ((uint32_t)raw_data[4]); //change to right values
      
      cs_conversion_data = cs_conversion_data & 0xFFFFFF;

      float output_voltage_V = 0.0;
      
      /*Anything in the 4.9 V range is already in over-voltage, therefore the need for as precicse accuracy is negligible at that point*/
      if ((cs_conversion_data == 0x800000 || cs_conversion_data == 0x800001 || cs_conversion_data == 0x7FFFFF)){
         output_voltage_V = csense_FSR;
      }else{
        if(negative){
          cs_conversion_data = ~ cs_conversion_data;
          cs_conversion_data++;
          cs_conversion_data = cs_conversion_data & 0xFFFFFF;
        }
        output_voltage_V = (float)(cs_conversion_data * csense_FSR) / (float)(1<<23);
      }  

      if(negative){
        output_voltage_V *= -1;
      }

      /* CURRENT */
      // float output_current_A = output_voltage_V / 0.0005;

      /* VOLTAGE */
      float csense_HV_voltage_V = output_voltage_V * (1000000 + 20000) / 20000;


      delay_ms(LOG_DB_DELAY);
      LOG_DEBUG("Output_voltage_V: %f | voltage: %f\r\n", output_voltage_V, csense_HV_volate_V); //VOLTAGE
      // LOG_DEBUG("Output_voltage_V: %f | current: %f\r\n", output_voltage_V, output_current_A); //CURRENT

      }
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
  interrupt_init();
  tasks_init();
  log_init();

  tasks_init_task(current_sense_run_cycle, TASK_PRIORITY(3), NULL);
  tasks_start();


  LOG_DEBUG("exiting main?\r\n");
  return 0;
}
