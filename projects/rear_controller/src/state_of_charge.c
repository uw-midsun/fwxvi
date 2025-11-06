/************************************************************************************************
 * @file    state_of_charge.c
 *
 * @brief   State of charge source file
 *
 * @date    2025-10-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "soc_ekf_matlab.h"
#include "state_of_charge.h"
#include "state_of_charge_lut.h"

/** @brief Default simulation timestep [s] */
#define SOC_DEFAULT_DT_S (1.0)

/** @brief Default cell parameters */
#define SOC_DEFAULT_R0_CELL_OHM (0.015)
#define SOC_DEFAULT_R1_CELL_OHM (0.05)
#define SOC_DEFAULT_C1_CELL_FARAD (2000.0)

/** @brief Process noise covariance diagonal terms */
#define SOC_Q_PROC_DIAG_0 (1e-5)
#define SOC_Q_PROC_DIAG_3 (1e-5)

/** @brief Measurement noise covariance */
#define SOC_R_MEAS (1e-2)

/** @brief Rest detection thresholds */
#define SOC_REST_THRESH_DELTA_V (0.05)
#define SOC_REST_WINDOW_SAMPLES (100)

/** @brief EKF exponential smoothing factor */
#define SOC_ALPHA (0.05)

/** @brief Improved initialization constants */
#define SOC_INIT_IR_COMP_A (0.2f)
#define SOC_INIT_VOLTAGE_TRIM_PCT (0.05f)
#define SOC_INIT_SOC_DEFAULT (0.5f)

static RearControllerStorage *rear_controller_storage;

static inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

/**
 * @brief Convert per-cell OCV to SOC using linear interpolation.
 * @param[in] v_cell     Cell open-circuit voltage [V]
 * @param[in] SOC_table  Array of SOC values
 * @param[in] OCV_table  Array of OCV values
 * @param[in] N          Table size
 * @return SOC [0–1]
 */
static float ocv_to_soc_c(float v_cell, const float *SOC_table, const float *OCV_table, int N) {
  if (N < 2 || SOC_table == NULL || OCV_table == NULL) return 0.5f;

  if (v_cell <= OCV_table[N - 1]) {
    return SOC_table[0U];
  }
  if (v_cell >= OCV_table[0U]) {
    return SOC_table[N - 1];
  }

  int idx = 0;
  for (int i = 0; i < N - 1; ++i) {
    if (v_cell <= OCV_table[i] && v_cell >= OCV_table[i + 1]) {
      idx = i;
      break;
    }
  }

  float v0 = OCV_table[idx];
  float v1 = OCV_table[idx + 1];
  float s0 = SOC_table[idx];
  float s1 = SOC_table[idx + 1];

  float soc_interp = s0 + (s1 - s0) * ((v_cell - v0) / (v1 - v0));

  return clamp01(soc_interp);
}

/**
 * @brief Estimate initial SOC and Vrc using startup voltage samples.
 */
void estimate_initial_state_c(const float *v_samples, int n_samples, int N_series, const float *SOC_table, const float *OCV_table, int table_size, float x0[2]) {
  if (v_samples == NULL || n_samples <= 0 || SOC_table == NULL || OCV_table == NULL || N_series <= 0) {
    x0[0] = SOC_INIT_SOC_DEFAULT;
    x0[1] = 0.0f;
    return;
  }

  float v_min = v_samples[0U];
  float v_max = v_samples[0U];
  float v_sum = 0.0f;
  for (int i = 0; i < n_samples; ++i) {
    float v = v_samples[i];
    if (v < v_min) v_min = v;
    if (v > v_max) v_max = v;
    v_sum += v;
  }

  float v_avg = v_sum / (float)n_samples;
  float v_trim_low = v_min + (v_max - v_min) * SOC_INIT_VOLTAGE_TRIM_PCT;
  float v_trim_high = v_max - (v_max - v_min) * SOC_INIT_VOLTAGE_TRIM_PCT;

  float v_sum_trimmed = 0.0f;
  int valid_count = 0;

  for (int i = 0; i < n_samples; ++i) {
    if (v_samples[i] >= v_trim_low && v_samples[i] <= v_trim_high) {
      v_sum_trimmed += v_samples[i];
      valid_count++;
    }
  }

  float V_pack = (valid_count > 0) ? v_sum_trimmed / valid_count : v_avg;
  float V_cell = V_pack / (float)N_series;

  float soc_est = ocv_to_soc_c(V_cell, SOC_table, OCV_table, table_size);

  x0[0] = clamp01(soc_est);
  x0[1] = 0.0f;
}

StatusCode state_of_charge_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  memset(rtU.x_prev, 0U, sizeof(rtU.x_prev));
  memset(rtU.P_prev, 0U, sizeof(rtU.P_prev));

  rtU.u_k = 0.0;
  rtU.z_k = 0.0;
  rtU.dt = SOC_DEFAULT_DT_S;

  rtU.params.N_series = rear_controller_storage->config->series_count;
  rtU.params.N_parallel = rear_controller_storage->config->parallel_count;
  rtU.params.Q_cell_Ah = rear_controller_storage->config->cell_capacity_Ah;

  rtU.params.R0_cell = SOC_DEFAULT_R0_CELL_OHM;
  rtU.params.R1_cell = SOC_DEFAULT_R1_CELL_OHM;
  rtU.params.C1_cell = SOC_DEFAULT_C1_CELL_FARAD;

  rtU.params.Q_pack_Ah = rtU.params.N_parallel * rtU.params.Q_cell_Ah;
  rtU.params.R0_pack = rtU.params.N_series * (rtU.params.R0_cell / rtU.params.N_parallel);
  rtU.params.R1_pack = rtU.params.N_series * (rtU.params.R1_cell / rtU.params.N_parallel);
  rtU.params.C1_pack = (rtU.params.N_parallel / rtU.params.N_series) * rtU.params.C1_cell;

  rtU.params.Q_proc[0U] = SOC_Q_PROC_DIAG_0;
  rtU.params.Q_proc[1U] = 0;
  rtU.params.Q_proc[2U] = 0;
  rtU.params.Q_proc[3U] = SOC_Q_PROC_DIAG_3;

  rtU.params.R_meas = SOC_R_MEAS;
  rtU.params.rest_thresh = SOC_REST_THRESH_DELTA_V;
  rtU.params.rest_window = SOC_REST_WINDOW_SAMPLES;
  rtU.params.alpha = SOC_ALPHA;

  rtU.params.SOC_OCV_table_size = SOC_OCV_TABLE_SIZE;

  const float soc_vals[SOC_OCV_TABLE_SIZE] = SOC_TABLE_VALUES;
  const float ocv_vals[SOC_OCV_TABLE_SIZE] = OCV_TABLE_VALUES;

  for (int i = 0; i < SOC_OCV_TABLE_SIZE; i++) {
    rtU.params.SOC_table[i] = (real_T)soc_vals[i];
    rtU.params.OCV_table[i] = (real_T)ocv_vals[i];
  }

  soc_ekf_matlab_initialize();

  float x0[2U];
  float v_samples[1U];
  int n_samples = 1U;

  v_samples[0U] = (float)(rear_controller_storage->pack_voltage) / 1000.0;

  estimate_initial_state_c(v_samples, n_samples, rtU.params.N_series, soc_vals, ocv_vals, SOC_OCV_TABLE_SIZE, x0);

  rtU.x_prev[0U] = x0[0U];
  rtU.x_prev[1U] = x0[1U];

  rear_controller_storage->estimated_state_of_charge = x0[0U];

  return STATUS_CODE_OK;
}

StatusCode state_of_charge_run() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  rtU.u_k = (real_T)(rear_controller_storage->pack_current) / 1000.0;
  rtU.z_k = (real_T)(rear_controller_storage->pack_voltage) / 1000.0;

  soc_ekf_matlab_step();

  rear_controller_storage->estimated_state_of_charge = rtY.x_new[0U];

  memcpy(rtU.x_prev, rtY.x_new, sizeof(rtY.x_new));
  memcpy(rtU.P_prev, rtY.P_new, sizeof(rtU.P_prev));

  return STATUS_CODE_OK;
}
