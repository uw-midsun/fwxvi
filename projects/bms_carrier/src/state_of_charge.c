/************************************************************************************************
 * @file   state_of_charge.h
 *
 * @brief  Header file for State of Charge
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "state_of_charge.h"

static struct StateOfChargeStorage state_of_charge_storage = {
  .averaged_soc = 0.0f,
  .i_soc = 0.0f,
  .v_soc = 0.0f,
  .last_current = 0,
  .last_time = 0U,
};

static BmsStorage *bms_storage;

static float voltage_weight = 1.0f;

/* Cell voltage. 0.2A Load taken off this page
 * https://lygte-info.dk/review/batteries2012/LG%2021700%20M50%205000mAh%20(Grey)%20UK.html */
/* Placeholder lookup for testing */
static float voltage_lookup[LUT_SIZE] = { 2.80, 3.00, 3.12, 3.18, 3.22, 3.25, 3.28, 3.30, 3.32, 3.34, 3.36, 3.38, 3.40, 3.42, 3.44, 3.46, 3.48, 3.50, 3.52, 3.54, 3.56,
                                          3.58, 3.60, 3.62, 3.64, 3.66, 3.68, 3.70, 3.72, 3.74, 3.76, 3.78, 3.80, 3.82, 3.84, 3.86, 3.89, 3.93, 3.98, 4.07, 4.15 };

// Ramps the voltage when its less than 30% SOC or greater than 70% SOC
// https://stackoverflow.com/questions/5731863/mapping-a-numeric-range-onto-another
void ramp_voltage_weight() {
  if (bms_storage->state_of_charge_storage->averaged_soc > 70.0f) {
    voltage_weight = 0.20f + ((0.6f / 30.0f) * (bms_storage->state_of_charge_storage->averaged_soc - 70.0f));
  } else if (bms_storage->state_of_charge_storage->averaged_soc < 30.0f) {
    voltage_weight = 0.20f + ((0.6f / 30.0f) * (30.0f - bms_storage->state_of_charge_storage->averaged_soc));
  } else {
    voltage_weight = 0.20f;
  }
}

static void update_storage() {
  bms_storage->state_of_charge_storage->last_current = bms_storage->pack_current;
  bms_storage->state_of_charge_storage->last_time = pdTICKS_TO_MS(xTaskGetTickCount());
}

float perdict_voltage() {
  // Voltage under load + Ohmic voltage drop in battery pack
  return (float)bms_storage->pack_voltage + (float)(bms_storage->pack_current * (PACK_INTERNAL_RESISTANCE_mOHMS / 1000.0f));
}

void coulomb_counting_soc() {
  float d_time = (float)(pdTICKS_TO_MS(xTaskGetTickCount()) - bms_storage->state_of_charge_storage->last_time) / (1000.0f * 3600.0f);

  // Trapezoidal rule
  float integrated_current = 0.5f * (float)(bms_storage->pack_current + bms_storage->state_of_charge_storage->last_current) * (d_time);
  bms_storage->state_of_charge_storage->i_soc = bms_storage->state_of_charge_storage->averaged_soc + (integrated_current / bms_storage->bms_config.pack_capacity);

  if (bms_storage->state_of_charge_storage->i_soc > 1.0f) {
    bms_storage->state_of_charge_storage->i_soc = 1.0f;
  } else if (bms_storage->state_of_charge_storage->i_soc < 0.0f) {
    bms_storage->state_of_charge_storage->i_soc = 0.0f;
  }
}

void ocv_voltage_soc() {
  uint8_t low_index = 0xFFU;
  uint8_t upper_index = 0;

  float pack_voltage = perdict_voltage();

  if (pack_voltage >= (voltage_lookup[LUT_SIZE - 1] * bms_storage->bms_config.series_count * VOLTS_TO_mV)) {
    bms_storage->state_of_charge_storage->v_soc = 100.0f;
    return;
  } else if (pack_voltage <= (voltage_lookup[0] * bms_storage->bms_config.series_count * VOLTS_TO_mV)) {
    bms_storage->state_of_charge_storage->v_soc = 0.0f;
    return;
  }

  for (uint8_t i = 0; i < LUT_SIZE - 1; i++) {
    if (pack_voltage >= (voltage_lookup[i] * bms_storage->bms_config.series_count * VOLTS_TO_mV) && pack_voltage <= (voltage_lookup[i + 1] * bms_storage->bms_config.series_count * VOLTS_TO_mV)) {
      low_index = i;
      upper_index = i + 1;
      break;
    }
  }

  if (low_index == 0xff || upper_index == 0) {
    return;
  }


  float voltage_low = voltage_lookup[low_index] * bms_storage->bms_config.series_count * VOLTS_TO_mV;
  float voltage_high = voltage_lookup[upper_index] * bms_storage->bms_config.series_count * VOLTS_TO_mV;
  // Lookup table index = SOC, voltage = value
  bms_storage->state_of_charge_storage->v_soc = low_index + ((float)(upper_index - low_index) * (pack_voltage - voltage_low) / (voltage_high - voltage_low));

  // We measure SOC over 2.5% increments
  // This means SOC = 2.5% * lookup_index
  bms_storage->state_of_charge_storage->v_soc *= (100.0f / (LUT_SIZE - 1));
}

StatusCode update_state_of_chrage() {
  coulomb_counting_soc();
  ocv_voltage_soc();

  bms_storage->state_of_charge_storage->averaged_soc = (voltage_weight * bms_storage->state_of_charge_storage->v_soc) + ((1 - voltage_weight) * (bms_storage->state_of_charge_storage->i_soc));

  set_battery_vt_batt_perc((uint16_t)bms_storage->state_of_charge_storage->averaged_soc);
  ramp_voltage_weight();
  update_storage();
  return STATUS_CODE_OK;
}

StatusCode state_of_charge_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  bms_storage = storage;
  bms_storage->state_of_charge_storage = &state_of_charge_storage;

  bms_storage->state_of_charge_storage->last_time = 0U;

  ocv_voltage_soc();

  bms_storage->state_of_charge_storage->i_soc = bms_storage->state_of_charge_storage->v_soc;
  bms_storage->state_of_charge_storage->averaged_soc = bms_storage->state_of_charge_storage->v_soc;

  ramp_voltage_weight();

  return STATUS_CODE_OK;
}

#if MS_TEST == 1U
void set_last_time(uint32_t last_time) {
  bms_storage->state_of_charge_storage->last_time = last_time;
}

void set_i_soc(float i_soc) {
  bms_storage->state_of_charge_storage->i_soc = i_soc;
}

void set_v_soc(float v_soc) {
  bms_storage->state_of_charge_storage->v_soc = v_soc;
}

void set_averaged_soc(float averaged_soc) {
  bms_storage->state_of_charge_storage->averaged_soc = averaged_soc;
}

void set_last_current(int32_t last_current) {
  bms_storage->state_of_charge_storage->last_current = last_current;
}

uint32_t get_last_time(void) {
  return bms_storage->state_of_charge_storage->last_time;
}

float get_i_soc(void) {
  return bms_storage->state_of_charge_storage->i_soc;
}

float get_v_soc(void) {
  return bms_storage->state_of_charge_storage->v_soc;
}

float get_averaged_soc(void) {
  return bms_storage->state_of_charge_storage->averaged_soc;
}

int32_t get_last_current(void) {
  return bms_storage->state_of_charge_storage->last_current;
}

float get_voltage_weight(void) {
  return voltage_weight;
}

#endif
