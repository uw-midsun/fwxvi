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
#include "rear_controller_setters.h"
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

static RearControllerStorage *rear_controller_storage;

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

  /* Run the generated step once so it computes the startup state (rtY.x0) from the pack voltage.
   * The EKF outputs from this call are ignored. */
  rtU.z_k = (real_T)rear_controller_storage->pack_voltage;
  soc_ekf_matlab_step();

  rtU.x_prev[0U] = rtY.x0[0U];
  rtU.x_prev[1U] = rtY.x0[1U];

  rear_controller_storage->estimated_state_of_charge = rtY.x0[0U];

  return STATUS_CODE_OK;
}

StatusCode state_of_charge_run() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  rtU.u_k = (real_T)rear_controller_storage->pack_current;
  rtU.z_k = (real_T)rear_controller_storage->pack_voltage;

  soc_ekf_matlab_step();

  rear_controller_storage->estimated_state_of_charge = rtY.x_new[0U];

  /* SOC EKF state is a 0-1 fraction; CAN pack_soc is a float percentage (0-100) */
  set_battery_stats_A_pack_soc(rear_controller_storage->estimated_state_of_charge * 100.0f);

  memcpy(rtU.x_prev, rtY.x_new, sizeof(rtY.x_new));
  memcpy(rtU.P_prev, rtY.P_new, sizeof(rtU.P_prev));

  return STATUS_CODE_OK;
}
