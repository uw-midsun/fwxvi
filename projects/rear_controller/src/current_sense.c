/************************************************************************************************
 * @file    precharge.c
 *
 * @brief   Motor precharge handler source file
 *
 * @date    2025-09-02
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "current_sense.h"

#include "current_acs37800.h"
#include "global_enums.h"
#include "status.h"

/* Intra-component Headers */
#include "bps_fault.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_setters.h"
#include "rear_controller_state_manager.h"

/* FSR = Vref / Gain -> Vref = 2.5, Gain = 0.5*/
#define csense_FSR 5
#define csense_AIN6_AIN7_MUX_CFG 0x67 /*shunt inputs*/
#define csense_AIN0_AIN1_MUX_CDF 0x01 /*HV_BUS and BAT_GND*/
#define csense_R6_ohm 1000000 /*1M ohm resistor*/
#define csense_R7_ohm 20000 /*20k ohm resistor*/


//if the input is above 3V it's above 153 V <- measured on HV_BUS
//TODO: CHANGE make sure that the conversion is reading the right data -> aka shunt isn't reading HV

#if (IS_USING_CURRENT_SENSE_REV_3 != 0U)
static float csense_current_A;
static float csense_HV_voltage_V;
static RearControllerStorage *rear_controller_storage;
static float csense_voltage_diff_V;
static float csense_shunt_resistance = 0.0005;
static int32_t csense_overcurrents = 0;
static int32_t csense_overvoltages = 0;
static int32_t csense_retries = 0;

static uint8_t register_map[] = {
  ADS122_REG_DEVICE_CFG_DEFAULT, //External or internal clock? -> Assuming internal
  ADS122_REG_DATA_RATE_CFG_DEFAULT, //Programmable conversion start delay selection, Global-chop mode, FLTR_OSR
  ADS122_REG_MUX_CFG_DEFAULT, // AINP and AINN are pin 0110b and 111b
  ADS122_REG_GAIN_CFG_DEFAULT, // Gain is 0.5
  (ADS122_REG_REFERENCE_CFG_DEFAULT | 0x04), //Vref = 2.5 V -> max range is +- 5 V, clock speed is 256 kHz
  ADS122_REG_DIGITAL_CFG_DEFAULT,  /* TODO: CHANGE BACK*/
  ADS122_REG_GPIO_CFG_DEFAULT,
  ADS122_REG_GPIO_DATA_OUTPUT_DEFAULT,
  ADS122_REG_IDAC_MAG_CFG_DEFAULT,
  ADS122_REG_IDAC_MUX_CFG_DEFAULT,
  ADS122_REG_REG_MAP_CRC_DEFAULT
};

StatusCode current_sense_init(RearControllerStorage * storage){
  if (storage == NULL) {
      return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  I2CSettings i2c_settings = {.speed = I2C_SPEED_FAST, .sda = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SDA_GPIO, .scl = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SCL_GPIO};

  // TODO: figure out if the slave address is correct
  status_ok_or_return(ads122_init(&storage->ads122_storage, REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, REAR_CONTOLLER_CURRENT_SENSE_ADC122_I2C_ADDR, register_map, &i2c_settings));

  return STATUS_CODE_OK;
}

static StatusCode csense_interpret_data(float * output_voltage, uint8_t MUX_CFG){
  static bool negative;
  static uint8_t cs_conversion_data_raw[3]; //TODO: change back
  static uint32_t cs_conversion_data;

  StatusCode status = ads122_get_conversion_data(&rear_controller_storage->ads122_storage, cs_conversion_data_raw, MUX_CFG);

  if (status != STATUS_CODE_OK) {
    if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
      csense_retries++;
      return STATUS_CODE_OK;
    } else {
      trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
      return STATUS_CODE_OK;
    }
  }

  negative = cs_conversion_data_raw[0] & 0x80;

  cs_conversion_data = ((uint32_t)cs_conversion_data_raw[0] << 16) | ((uint32_t)cs_conversion_data_raw[1] << 8) | ((uint32_t)cs_conversion_data_raw[2]);
  
  /*Anything in the 4.9 V range is already in over-voltage, therefore the need for as precicse accuracy is negligible at that point*/
  if (negative && (cs_conversion_data == 0x800000 || cs_conversion_data == 0x800001)){
    *output_voltage = csense_FSR;
  }else if (!negative && cs_conversion_data == 0x7FFFFF){
    *output_voltage = csense_FSR;
  }else{
    if(negative){
      cs_conversion_data = ~ cs_conversion_data;
      cs_conversion_data++;
    }
    *output_voltage = (float)(cs_conversion_data * csense_FSR) / (float)(1<<23);

  }  

  if(negative){
    *output_voltage *= -1;
  }

  return STATUS_CODE_OK;
}

StatusCode current_sense_run() {

  /* Current*/
  // StatusCode status = csense_interpret_data(&csense_voltage_diff_V, csense_AIN6_AIN7_MUX_CFG);

  // if (status != STATUS_CODE_OK) {
  //   if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
  //     csense_retries++;
  //     return STATUS_CODE_OK;
  //   } else {
  //     trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
  //     return STATUS_CODE_OK;
  //   }
  // }

  // csense_current_A = csense_shunt_resistance * csense_voltage_diff_V;

  // if(csense_current_A < PACK_MAX_DISCHARGE_CURRENT_A || csense_current_A > PACK_MAX_CHARGE_CURRENT_A){
  //   csense_overcurrents++;
  //   if(csense_overcurrents > OVERCURRENT_RESPONSE_LOOPS){
  //     trigger_bps_fault(BPS_FAULT_OVERCURRENT);
  //   }
  // }

  /* Voltage */
  StatusCode status = csense_interpret_data(&csense_HV_voltage_V, csense_AIN0_AIN1_MUX_CDF);

  if (status != STATUS_CODE_OK) {
    if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
      csense_retries++;
      return STATUS_CODE_OK;
    } else {
      trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
      return STATUS_CODE_OK;
    }
  }

  csense_HV_voltage_V *= (csense_R6_ohm + csense_R7_ohm) / csense_R7_ohm;
  
  if (csense_HV_voltage_V > PACK_OVERVOLTAGE_LIMIT_mV * 0.001) {
    csense_overvoltages++;
    if (csense_overvoltages > OVERCURRENT_RESPONSE_LOOPS) {
      trigger_bps_fault(BPS_FAULT_OVERVOLTAGE);
    }
  } else {
    csense_overvoltages = 0;
  }

  /* Update rear_controller_storage with voltage and current*/
  rear_controller_storage->pack_current = (int32_t)(csense_current_A * 1000.0f); 
  rear_controller_storage->pack_voltage = (uint32_t)(csense_HV_voltage_V * 1000.0f);

  set_battery_stats_A_pack_current((int16_t)rear_controller_storage->pack_current);
  set_battery_stats_A_pack_voltage((int16_t)rear_controller_storage->pack_voltage);
  
  return STATUS_CODE_OK;
}




#else
  static float csense_prev_current_A;
  static float csense_prev_voltage_mV;
  static int32_t csense_overcurrents;
  static int32_t csense_overvoltages;
  static int32_t csense_retries;

  static RearControllerStorage *rear_controller_storage;

  // https://blog.mbedded.ninja/programming/signal-processing/digital-filters/exponential-moving-average-ema-filter/.
  float filter_step(const float alpha, float x, float prev_y) {
    return alpha * x + (1 - alpha) * prev_y;
  }

  StatusCode current_sense_run() {
    float current_reading_A;
    float voltage_reading_mV = 0.0f;

    /* Check current */
    StatusCode status = acs37800_get_current(&rear_controller_storage->acs37800_storage, &current_reading_A);

    if (status != STATUS_CODE_OK) {
      if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
        csense_retries++;
        return STATUS_CODE_OK;
      } else {
        trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
        return STATUS_CODE_OK;
      }
    }

    csense_retries = 0;

    float current_A = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, current_reading_A, csense_prev_current_A);

    if (current_A < PACK_MAX_DISCHARGE_CURRENT_A || current_A > PACK_MAX_CHARGE_CURRENT_A) {
      csense_overcurrents++;
      if (csense_overcurrents > OVERCURRENT_RESPONSE_LOOPS) {
        trigger_bps_fault(BPS_FAULT_OVERCURRENT);
      }
    } else {
      csense_overcurrents = 0;
    }

    /* Check voltage */
    status = acs37800_get_voltage(&rear_controller_storage->acs37800_storage, &voltage_reading_mV);

    if (status != STATUS_CODE_OK) {
      if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
        csense_retries++;
        return STATUS_CODE_OK;
      } else {
        trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
        return STATUS_CODE_OK;
      }
    }

    csense_retries = 0;

    float voltage_mV = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, voltage_reading_mV, csense_prev_voltage_mV);

    if (voltage_mV > PACK_OVERVOLTAGE_LIMIT_mV) {
      csense_overvoltages++;
      if (csense_overvoltages > OVERCURRENT_RESPONSE_LOOPS) {
        trigger_bps_fault(BPS_FAULT_OVERVOLTAGE);
      }
    } else {
      csense_overvoltages = 0;
    }

    /* Store current and voltage in mA and mV respectively */
    rear_controller_storage->pack_current = (int32_t)(current_A * 1000.0f);
    rear_controller_storage->pack_voltage = (uint32_t)(voltage_reading_mV);

    set_battery_stats_A_pack_current((int16_t)rear_controller_storage->pack_current);
    set_battery_stats_A_pack_voltage((int16_t)rear_controller_storage->pack_voltage);

    csense_prev_current_A = current_A;
    csense_prev_voltage_mV = voltage_mV;

    return STATUS_CODE_OK;
  }

  StatusCode current_sense_init(RearControllerStorage *storage) {
    if (storage == NULL) {
      return STATUS_CODE_INVALID_ARGS;
    }

    rear_controller_storage = storage;

    status_ok_or_return(acs37800_init(&rear_controller_storage->acs37800_storage, REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, REAR_CONTROLLER_CURRENT_SENSE_ACS37800_I2C_ADDR));

    return STATUS_CODE_OK;
  }
#endif