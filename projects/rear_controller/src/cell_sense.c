/************************************************************************************************
 * @file   cell_sense.c
 *
 * @brief  Source code for AFE cell sense
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adbms_afe.h"
#include "delay.h"
#include "global_enums.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "bms_hw_defs.h"
#include "cell_sense.h"
#include "rear_controller.h"

/************************************************************************************************
 * Private defines
 ************************************************************************************************/

/** @brief  Number of AFE messages */
#define NUM_AFE_MSGS 4U
/** @brief  Number of cell voltage readings per AFE message */
#define READINGS_PER_AFE_MSG 3U
/** @brief  Thermistor resistance value */
#define TEMP_RESISTANCE 10000
/** @brief  Voltage reference for ADBMS1818 ADCs in 100uV */
#define VREF2 30000.0f
/** @brief  Voltage gain of ADBMS1818 ADCs in 100uV / sample */
#define ADC_GAIN (VREF2 / ((1 << 15) - 1))
/** @brief  Thermistor resistance lookup table size */
#define TABLE_SIZE 125U
/** @brief  Number of communication retries before throwing AFE fault */
#define AFE_NUM_RETRIES 5U
/** @brief  Temperature threshold for invalid readings */
#define CELL_TEMP_OUTLIER_THRESHOLD 80
/** @brief  Maximum pack current - 7.0A -> 7000mA */
#define PACK_OVERCURRENT_CURRENT 7000
/** @brief  Private define to lookup cell voltage */
#define CELL_PER_DEVICE 16
#define CELL_VOLTAGE_LOOKUP(dev_num, cell) (adbms_afe_storage->cell_voltages[adbms_afe_storage->cell_result_lookup[CELL_PER_DEVICE * dev_num + cell]])
/** @brief  Max number of retries for reading cell*/
#define BMS_MAX_RETRIES 5U

/**
 * @brief   Thermistor mapping for the ADBMS1818 hardware schematic
 */
typedef enum ThermistorMap { THERMISTOR_2 = 0, THERMISTOR_1, THERMISTOR_0, THERMISTOR_3, THERMISTOR_4, THERMISTOR_7, THERMISTOR_5, THERMISTOR_6, NUM_THERMISTORS } ThermistorMap;
                                                          
#define RETRY_OPERATION(max_retries, delay_ms_val, operation, status_var) \
  do {                                                                    \
    uint8_t _retries_left = (max_retries);                                \
    (status_var) = STATUS_CODE_INTERNAL_ERROR;                            \
    while (_retries_left-- > 0) {                                         \
      (status_var) = (operation);                                         \
      if ((status_var) == STATUS_CODE_OK) break;                          \
      delay_ms(delay_ms_val);                                             \
    }                                                                     \
  } while (0)
  
/************************************************************************************************
 * Private variables
 ************************************************************************************************/

/* Forward definition */

int calculate_temperature(uint16_t thermistor);

SpiSettings spi_test_settings = {
  .baudrate = ADBMS_AFE_SPI_BAUDRATE,
  .mode = SPI_MODE_3,
  .sdo = ADBMS_AFE_SPI_SDO_GPIO,
  .sdi = ADBMS_AFE_SPI_SDI_GPIO,
  .sclk = ADBMS_AFE_SPI_SCK_GPIO,
  .cs = ADBMS_AFE_SPI_CS_GPIO,
};

static const AdbmsAfeSettings s_afe_settings = {
  .spi_settings = &spi_test_settings,

  .spi_port = ADBMS_AFE_SPI_PORT,

  .adc_mode = ADBMS_AFE_ADC_MODE_7KHZ,

  .cell_bitset = { 0xFFFU, 0xFFFU, 0xFFFU },

  .num_devices = 2U,
  .num_cells = 18U,
  .num_thermistors = NUM_THERMISTORS,
};

static AdbmsAfeStorage *adbms_afe_storage;

static bool s_cell_data_updated = false;

uint8_t afe_message_index = 0U;

static uint8_t s_thermistor_map[NUM_THERMISTORS] = {
  [0] = THERMISTOR_0, [1] = THERMISTOR_1, [2] = THERMISTOR_2, [3] = THERMISTOR_3, [4] = THERMISTOR_4, [5] = THERMISTOR_5, [6] = THERMISTOR_6, [7] = THERMISTOR_7
};

static RearControllerStorage *rear_controller_storage;

// Murata NCP15XH103F03RC 10k NTC, B(25/50)=3380 K
// Index = °C (0, 0.5, 1, ..,  124), Value = resistance (ohms)
static const uint16_t s_resistance_lookup[249] = {
  28224,        27593,  26978,  26379,  25796,  25228,  24674,  24134,  23608,  23095,  22595,  22108,  21632,  21169,      20717,  20277,  19847,  19428,  19019,  18620,  18231,  17852,  17481,  17120,
  16767,        16423,  16087,  15759,  15438,  15126,  14820,  14522,  14231,  13947,  13669,  13398,  13133,  12874,      12622,  12375,  12133,  11897,  11667,  11441,  11221,  11006,  10796,  10590,
  10389,        10192,  10000,  9812,   9628,   9448,   9272,   9100,   8932,   8767,   8606,   8449,   8295,   8144,       7996,   7852,   7710,   7572,   7436,   7303,   7174,   7046,   6922,   6800,
  6681, 6564,   6449,   6337,   6227,   6120,   6014,   5911,   5810,   5711,   5614,   5518,   5425,   5334,   5244,       5156,   5070,   4986,   4903,   4822,   4743,   4665,   4589,   4514,
  4440, 4368,   4297,   4228,   4160,   4093,   4028,   3964,   3901,   3839,   3779,   3719,   3661,   3603,   3547,       3492,   3438,   3385,   3333,   3281,   3231,   3182,   3133,   3086,
  3039, 2993,   2948,   2904,   2861,   2818,   2776,   2735,   2695,   2655,   2616,   2577,   2540,   2503,   2467,       2431,   2396,   2361,   2327,   2294,   2261,   2229,   2197,   2166,
  2136, 2106,   2076,   2047,   2019,   1991,   1963,   1936,   1909,   1883,   1857,   1832,   1807,   1782,   1758,       1734,   1711,   1688,   1665,   1643,   1621,   1600,   1578,   1558,
  1537, 1517,   1497,   1477,   1458,   1439,   1420,   1402,   1384,   1366,   1349,   1331,   1315,   1298,   1281,       1265,   1249,   1233,   1218,   1203,   1188,   1173,   1158,   1144,
  1130, 1116,   1102,   1089,   1076,   1062,   1050,   1037,   1024,   1012,   1000,   988,    976,    964,    953,        942,     930,    920,    909,    898,    888,    877,    867,    857,
  847,  837,    828,    818,    809,    799,    790,    781,    772,    764,    755,    747,    738,    730,    722,        714,     706,    698,    690,    683,    675,    668,    660,    653,
  646,  639,    632,    625,    619,    612,    605,    599,    593
};

/************************************************************************************************
 * Private function definitions
 ************************************************************************************************/

static void s_balance_cells(uint16_t min_voltage) {
  uint16_t balancing_threshold = min_voltage;
  if(rear_controller_storage->pack_current > PACK_OVERCURRENT_CURRENT) //don't balance
    return;
  /* Adjust balancing threshold */
  if (min_voltage >= AFE_BALANCING_UPPER_THRESHOLD) {
    /* If the minimum cell voltage is greater than 4.15V, we want minimum 2mV imbalance */
    balancing_threshold += 20U;
  } else if (min_voltage < AFE_BALANCING_UPPER_THRESHOLD && balancing_threshold >= AFE_BALANCING_LOWER_THRESHOLD) {
    /* If the minimum cell voltage is between 4.0V and 4.15V, we want minimum 5mV imbalance */
    balancing_threshold += 50U;
  } else {
    /* If the minimum cell voltage is less than 4.0V, we want minimum 10mV imbalance */
    balancing_threshold += 100U;
  }

  /* Toggle cell discharge in the ADBMS1818 configuration if cell voltage is above the balancing threshold */
  for(size_t dev = 0U; dev < s_afe_settings.num_devices; dev++){
    for (size_t cell = 0U; cell <  s_afe_settings.num_cells; cell++) {
      if (CELL_VOLTAGE_LOOKUP(dev, cell) > balancing_threshold) {
        adbms_afe_toggle_cell_discharge(adbms_afe_storage, cell * dev, true);
      } else {
        adbms_afe_toggle_cell_discharge(adbms_afe_storage, cell * dev, false);
      }
    }
  }

  /* Commit the discharge configuration to the ADBMS1818 */
  adbms_afe_write_config(adbms_afe_storage);
}

static StatusCode s_check_thermistors() {
  StatusCode status = STATUS_CODE_OK;
  uint16_t max_temp = 0U;

  /* Loop over all devices and thermistors */
  for (uint8_t device = 0U; device < s_afe_settings.num_devices; device++) {
    for (uint8_t thermistor = 0U; thermistor < ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE; thermistor++) {
      /* Check if the temperature ADC is enabled for the provided device and thermistor channel */
      if ((s_afe_settings.cell_bitset[device] >> thermistor) & 1U) {
        /* Calculate the ADC voltage index given the device and thermistor number */
        uint8_t index = device * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor;
        adbms_afe_storage->cell_voltages[index] = calculate_temperature(adbms_afe_storage->cell_voltages[index]);
        //TODO: make this division floating point
        LOG_DEBUG("Thermistor reading device %d, %d: %d\n", device, thermistor, adbms_afe_storage->cell_voltages[index]/10);

        /* Ignore temperature readings outside of the valid temperature range */
        if (adbms_afe_storage->cell_voltages[index] > CELL_TEMP_OUTLIER_THRESHOLD * 10) {
          continue;
        }

        if (adbms_afe_storage->cell_voltages[index] > max_temp * 10) {
          max_temp = adbms_afe_storage->cell_voltages[index];
        }

        delay_ms(3);
        if (rear_controller_storage->pack_current < 0) {
          if (adbms_afe_storage->cell_voltages[index] >= CELL_MAX_TEMPERATURE_DISCHARGE * 10) {
            LOG_DEBUG("CELL OVERTEMP\n");
            // fault_bps_set(BMS_FAULT_OVERTEMP_CELL);
            status = STATUS_CODE_INTERNAL_ERROR;
          }
        } else {
          if (adbms_afe_storage->cell_voltages[index] >= CELL_MAX_TEMPERATURE_CHARGE * 10) {
            LOG_DEBUG("CELL OVERTEMP\n");
            // fault_bps_set(BMS_FAULT_OVERTEMP_CELL);
            status = STATUS_CODE_INTERNAL_ERROR;
          }
        }
      }
    }
  }

  // rear_controller_storage->max_temperature = max_temp;

  return status;
}

static StatusCode s_cell_sense_conversions() {
  StatusCode status = STATUS_CODE_OK;
  // TODO: Figure out why cell_conv cannot happen without spi timing out (Most likely RTOS
  // implemntation error) Retry Mechanism
  for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
    // status = adbms_afe_trigger_cell_conv(adbms_afe_storage); 
    RETRY_OPERATION(BMS_MAX_RETRIES, RETRY_DELAY_MS, adbms_afe_trigger_cell_conv(adbms_afe_storage),
                  status);
    if (status == STATUS_CODE_OK) {
      break;
    }
    LOG_DEBUG("Cell trigger conv failed, retrying %d\n", status);
    delay_ms(RETRY_DELAY_MS);
  }
  if (status) {
    // If this has failed, try once more after a short delay
    LOG_DEBUG("Cell trigger conv failed, retrying): %d\n", status);
    delay_ms(RETRY_DELAY_MS);
    // status = adbms_afe_trigger_cell_conv(adbms_afe_storage);
    RETRY_OPERATION(BMS_MAX_RETRIES, RETRY_DELAY_MS, adbms_afe_trigger_cell_conv(adbms_afe_storage), status);
  }
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell conv failed): %d\n", status);
    // fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return status;
  }
  delay_ms(CONV_DELAY_MS);

  for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
    status = adbms_afe_read_cells(adbms_afe_storage);
    if (status == STATUS_CODE_OK) {
      break;
    }
    LOG_DEBUG("Cell read failed, retrying %d\n", status);
    delay_ms(RETRY_DELAY_MS);
  }
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell read failed %d\n", status);
    // fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return status;
  }

  for (size_t thermistor = 0; thermistor < s_afe_settings.num_thermistors; thermistor += 1) {
    // Thermistor indexes are read for each daisy chained dev at the same time
    // Check therm bitset to determine if we need to read any at this index
    bool check_therm = false;
    for (uint8_t dev = 0; dev < s_afe_settings.num_devices; dev++) {
      if ((s_afe_settings.cell_bitset[dev] >> thermistor) & 0x1) {
        check_therm = true;
      }
    }

    if (check_therm) {
      // Trigger and read thermistor value
      for(uint8_t dev = 0; dev < s_afe_settings.num_devices; dev++){
        for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
          status = adbms_afe_trigger_thermistor_conv(adbms_afe_storage, dev, thermistor);
          if (status == STATUS_CODE_OK) {
            break;
          }
          LOG_DEBUG("Aux trigger conv failed, retrying: %d\n", status);
          delay_ms(RETRY_DELAY_MS);
        }
        if (status) {
          LOG_DEBUG("Thermistor conv failed for therm %d: Status %d\n", (uint8_t)thermistor, status);
          // fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
          return status;
        }
        delay_ms(AUX_CONV_DELAY_MS);

        for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
          status = adbms_afe_read_thermistor(adbms_afe_storage, dev, thermistor);      
          if (status == STATUS_CODE_OK) {
            break;
          }
          LOG_DEBUG("Thermistor read failed, retrying %d\n", status);
          delay_ms(RETRY_DELAY_MS);
        }
        if (status) {
          LOG_DEBUG("Thermistor read trigger failed for thermistor %d,  %d\n", (uint8_t)thermistor, status);
          // fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
          return status;
        }
      }
    }
  }
  return status;
}

static StatusCode s_cell_sense_run() {
  StatusCode status = STATUS_CODE_OK;

  uint16_t max_voltage = 0U;
  uint16_t min_voltage = 0xFFFFU;
  for(size_t dev = 0U; dev < s_afe_settings.num_devices; dev ++){
    for (size_t cell = 0U; cell < s_afe_settings.num_cells; cell++) {
      uint16_t current_cell_voltage = CELL_VOLTAGE_LOOKUP(dev, cell);
      LOG_DEBUG("CELL %d: %d\n\r", (uint8_t)cell, current_cell_voltage);
      delay_ms(5U);
      if (current_cell_voltage > max_voltage) {
        max_voltage = current_cell_voltage;
      }
      if (current_cell_voltage < min_voltage) {
        min_voltage = current_cell_voltage;
      }
    }
  }
  delay_ms(10);

  LOG_DEBUG("MAX VOLTAGE: %d\n", max_voltage);
  LOG_DEBUG("MIN VOLTAGE: %d\n", min_voltage);
  LOG_DEBUG("UNBALANCE: %d\n", max_voltage - min_voltage);
  // set_battery_info_max_cell_v(max_voltage);
  // set_battery_info_min_cell_v(min_voltage);

  if (max_voltage >= SOLAR_VOLTAGE_THRESHOLD) {
    // bms_open_solar();
    rear_controller_storage->solar_relay_closed = false;
  }

  if (max_voltage >= CELL_OVERVOLTAGE) {
    LOG_DEBUG("OVERVOLTAGE\n");
    // fault_bps_set(BMS_FAULT_OVERVOLTAGE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }
  if (min_voltage <= CELL_UNDERVOLTAGE) {
    LOG_DEBUG("UNDERVOLTAGE\n");
    // fault_bps_set(BMS_FAULT_UNDERVOLTAGE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }
  if (max_voltage - min_voltage >= CELL_UNBALANCED) {
    LOG_DEBUG("UNBALANCED\n");
    // fault_bps_set(BMS_FAULT_UNBALANCE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }

  s_balance_cells(min_voltage);

  status_ok_or_return(s_check_thermistors());

  s_cell_data_updated = true;

  return status;
}

/************************************************************************************************
 * Public function definitions
 ************************************************************************************************/
//returns in units of (0.1C) -> thermistor = 5 means 0.5 C
int calculate_temperature(uint16_t thermistor) {
  thermistor = (uint16_t)(thermistor * ADC_GAIN);                                          // 100uV
  uint16_t thermistor_resistance = (thermistor * TEMP_RESISTANCE) / (VREF2 - thermistor);  // Ohms
  delay_ms(10);
  uint16_t min_diff = abs(thermistor_resistance - s_resistance_lookup[0]);

  for (uint8_t i = 1U; i < TABLE_SIZE; ++i) {
    if (abs(thermistor_resistance - s_resistance_lookup[i]) < min_diff) {
      min_diff = abs(thermistor_resistance - s_resistance_lookup[i]);
      thermistor = i * 5;
    }
  }
  return thermistor;
}

StatusCode log_cell_sense() {
  /* AFE messages are split into 3 (For each AFE) */
  /* We send 4 messages total to transmit all cell voltages (3 per msg * 4 times) = 12 total cell voltages */
  if (s_cell_data_updated != true) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  // uint8_t read_index = afe_message_index * READINGS_PER_AFE_MSG;
  // set_AFE1_status_id(afe_message_index);
  // AFE1_status_A(afe_message_index, CELL_VOLTAGE_LOOKUP(0, 0), CELL_VOLTAGE_LOOKUP(0, 1), CELL_VOLTAGE_LOOKUP(0, 2), CELL_VOLTAGE_LOOKUP(0, 3));
  // set_AFE1_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  // set_AFE1_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  // set_AFE1_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  // read_index = (uint8_t)s_afe_settings.num_cells + afe_message_index * READINGS_PER_AFE_MSG;
  // AFE1_status_B(afe_message_index, CELL_VOLTAGE_LOOKUP(1, 0), CELL_VOLTAGE_LOOKUP(1, 1), CELL_VOLTAGE_LOOKUP(1, 2), CELL_VOLTAGE_LOOKUP(1, 3));
  // set_AFE2_status_id(afe_message_index);
  // set_AFE2_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  // set_AFE2_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  // set_AFE2_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  // read_index = (uint8_t)s_afe_settings.num_cells * 2 + afe_message_index * READINGS_PER_AFE_MSG;
  // set_AFE3_status_id(afe_message_index);
  // set_AFE3_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  // set_AFE3_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  // set_AFE3_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  // AFE_temperature(?)
  // // Thermistors to send are at index 0, 2, 4 for each device
  // if (afe_message_index < NUM_AFE_MSGS - 1) {  // Only 3 thermistors per device, so 4th message will be ignored
  //   set_AFE1_status_temp(adbms_afe_storage->cell_voltages[afe_message_index * 2]/10);
  //   set_AFE2_status_temp(adbms_afe_storage->cell_voltages[ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + afe_message_index * 2]/10);
  //   set_AFE3_status_temp(adbms_afe_storage->cell_voltages[ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE * 2 + afe_message_index * 2]/10);
  // }

  afe_message_index = (afe_message_index + 1) % NUM_AFE_MSGS;

  if (afe_message_index == 0) {
    s_cell_data_updated = false;
  }

  return STATUS_CODE_OK;
}

TASK(cell_sense_conversions, TASK_STACK_512) {
  adbms_afe_init(adbms_afe_storage, &s_afe_settings);
  delay_ms(10);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (true) {
    s_cell_sense_conversions();
    s_cell_sense_run();
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000U));
  }
}

StatusCode cell_sense_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  cell_sense_init(storage);

  rear_controller_storage = storage;
  adbms_afe_storage = &(rear_controller_storage->adbms_afe_storage);
  tasks_init_task(cell_sense_conversions, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}
