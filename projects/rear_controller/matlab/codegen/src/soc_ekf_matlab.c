/************************************************************************************************
 * @file    soc_ekf_matlab.c
 *
 * @brief   Soc Ekf Matlab
 *
 * @date    2025-10-31
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "rtwtypes.h"

/* Intra-component Headers */
#include "soc_ekf_matlab.h"

/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: soc_ekf_matlab.c
 *
 * Code generated for Simulink model 'soc_ekf_matlab'.
 *
 * Model version                  : 7.0
 * Simulink Coder version         : 26.2 (R2026b) 22-May-2026
 * C/C++ source code generated on : Tue Nov 17 10:03:36 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;

/* Model step function */
void soc_ekf_matlab_step(void) {
  real_T Fd_0;
  real_T H_idx_0;
  real_T P_new_tmp;
  real_T P_new_tmp_0;
  real_T kalman_gain_idx_0;
  real_T rtb_x_new_idx_1;
  int32_T d_idx;
  int32_T i;
  int8_T Fd[4];

  /* MATLAB Function: '<Root>/soc_ekf_matlab' incorporates:
   *  Inport: '<Root>/P_prev'
   *  Inport: '<Root>/dt'
   *  Inport: '<Root>/params'
   *  Inport: '<Root>/u_k'
   *  Inport: '<Root>/x_prev'
   *  Inport: '<Root>/z_k'
   */
  rtDW.b_idx = rtU.params.R1_pack * rtU.params.C1_pack;
  if (rtDW.b_idx <= 0.0) {
    rtDW.tau = 0.0;
  } else {
    rtDW.tau = exp(-rtU.dt / rtDW.b_idx);
  }

  rtDW.rtb_x_new_idx_0 = fmin(fmax(rtU.u_k * rtU.dt / (rtU.params.Q_pack_Ah * 3600.0) + rtU.x_prev[0], 0.0), 1.0);
  rtb_x_new_idx_1 = (1.0 - rtDW.tau) * rtU.params.R1_pack * rtU.u_k + rtDW.tau * rtU.x_prev[1];
  if (rtDW.b_idx <= 0.0) {
    rtDW.tau = 0.0;
  } else {
    rtDW.tau = exp(-rtU.dt / rtDW.b_idx);
  }

  rtDW.Fd[0] = 0.0;
  rtDW.Fd[1] = 0.0;
  rtDW.Fd[2] = 0.0;
  rtDW.Fd[3] = 0.0;
  for (d_idx = 0; d_idx < 2; d_idx++) {
    i = d_idx << 1;
    P_new_tmp = rtU.P_prev[i];
    rtDW.soc_m = rtDW.Fd[i + 1] + 0.0 * P_new_tmp;
    P_new_tmp_0 = rtU.P_prev[i + 1];
    rtDW.Fd[i] = (rtDW.Fd[i] + P_new_tmp) + 0.0 * P_new_tmp_0;
    rtDW.Fd[i + 1] = rtDW.tau * P_new_tmp_0 + rtDW.soc_m;
  }

  for (d_idx = 0; d_idx < 2; d_idx++) {
    rtDW.soc_m = rtDW.Fd[d_idx + 2];
    rtDW.idx = rtDW.Fd[d_idx];
    rtDW.P_pred[d_idx] = (rtDW.soc_m * 0.0 + rtDW.idx) + rtU.params.Q_proc[d_idx];
    rtDW.P_pred[d_idx + 2] = (rtDW.soc_m * rtDW.tau + rtDW.idx * 0.0) + rtU.params.Q_proc[d_idx + 2];
  }

  rtDW.tau = fmin(rtDW.rtb_x_new_idx_0 + 1.0E-5, 1.0);
  rtDW.soc_m = fmax(rtDW.rtb_x_new_idx_0 - 1.0E-5, 0.0);
  if (1.0 - rtDW.tau <= rtU.params.SOC_table[0]) {
    rtDW.idx = rtU.params.OCV_table[0];
  } else if (1.0 - rtDW.tau >= rtU.params.SOC_table[(int32_T)rtU.params.SOC_OCV_table_size - 1]) {
    rtDW.idx = rtU.params.OCV_table[(int32_T)rtU.params.SOC_OCV_table_size - 1];
  } else {
    rtDW.idx = -1.0;
    d_idx = 0;
    while (d_idx <= (int32_T)(rtU.params.SOC_OCV_table_size - 1.0) - 1) {
      if ((1.0 - rtDW.tau >= rtU.params.SOC_table[d_idx]) && (1.0 - rtDW.tau <= rtU.params.SOC_table[d_idx + 1])) {
        rtDW.idx = (real_T)d_idx + 1.0;
        break;
      } else {
        d_idx++;
      }
    }

    if (rtDW.idx == -1.0) {
      rtDW.idx = rtU.params.SOC_OCV_table_size - 1.0;
    }

    P_new_tmp = rtU.params.SOC_table[(int32_T)rtDW.idx - 1];
    P_new_tmp_0 = rtU.params.SOC_table[(int32_T)(rtDW.idx + 1.0) - 1] - P_new_tmp;
    if (P_new_tmp_0 == 0.0) {
      rtDW.idx = rtU.params.OCV_table[(int32_T)rtDW.idx - 1];
    } else {
      rtDW.b_idx = rtU.params.OCV_table[(int32_T)rtDW.idx - 1];
      rtDW.idx = ((1.0 - rtDW.tau) - P_new_tmp) / P_new_tmp_0 * (rtU.params.OCV_table[(int32_T)(rtDW.idx + 1.0) - 1] - rtDW.b_idx) + rtDW.b_idx;
    }
  }

  if (1.0 - rtDW.soc_m <= rtU.params.SOC_table[0]) {
    rtDW.b_idx = rtU.params.OCV_table[0];
  } else if (1.0 - rtDW.soc_m >= rtU.params.SOC_table[(int32_T)rtU.params.SOC_OCV_table_size - 1]) {
    rtDW.b_idx = rtU.params.OCV_table[(int32_T)rtU.params.SOC_OCV_table_size - 1];
  } else {
    rtDW.b_idx = -1.0;
    d_idx = 0;
    while (d_idx <= (int32_T)(rtU.params.SOC_OCV_table_size - 1.0) - 1) {
      if ((1.0 - rtDW.soc_m >= rtU.params.SOC_table[d_idx]) && (1.0 - rtDW.soc_m <= rtU.params.SOC_table[d_idx + 1])) {
        rtDW.b_idx = (real_T)d_idx + 1.0;
        break;
      } else {
        d_idx++;
      }
    }

    if (rtDW.b_idx == -1.0) {
      rtDW.b_idx = rtU.params.SOC_OCV_table_size - 1.0;
    }

    P_new_tmp = rtU.params.SOC_table[(int32_T)rtDW.b_idx - 1];
    P_new_tmp_0 = rtU.params.SOC_table[(int32_T)(rtDW.b_idx + 1.0) - 1] - P_new_tmp;
    if (P_new_tmp_0 == 0.0) {
      rtDW.b_idx = rtU.params.OCV_table[(int32_T)rtDW.b_idx - 1];
    } else {
      kalman_gain_idx_0 = rtU.params.OCV_table[(int32_T)rtDW.b_idx - 1];
      rtDW.b_idx = ((1.0 - rtDW.soc_m) - P_new_tmp) / P_new_tmp_0 * (rtU.params.OCV_table[(int32_T)(rtDW.b_idx + 1.0) - 1] - kalman_gain_idx_0) + kalman_gain_idx_0;
    }
  }

  H_idx_0 = (rtU.params.N_series * rtDW.idx - rtU.params.N_series * rtDW.b_idx) / ((rtDW.tau - rtDW.soc_m) + 1.0E-12);
  if (1.0 - rtDW.rtb_x_new_idx_0 <= rtU.params.SOC_table[0]) {
    rtDW.tau = rtU.params.OCV_table[0];
  } else if (1.0 - rtDW.rtb_x_new_idx_0 >= rtU.params.SOC_table[(int32_T)rtU.params.SOC_OCV_table_size - 1]) {
    rtDW.tau = rtU.params.OCV_table[(int32_T)rtU.params.SOC_OCV_table_size - 1];
  } else {
    rtDW.soc_m = -1.0;
    d_idx = 0;
    while (d_idx <= (int32_T)(rtU.params.SOC_OCV_table_size - 1.0) - 1) {
      if ((1.0 - rtDW.rtb_x_new_idx_0 >= rtU.params.SOC_table[d_idx]) && (1.0 - rtDW.rtb_x_new_idx_0 <= rtU.params.SOC_table[d_idx + 1])) {
        rtDW.soc_m = (real_T)d_idx + 1.0;
        break;
      } else {
        d_idx++;
      }
    }

    if (rtDW.soc_m == -1.0) {
      rtDW.soc_m = rtU.params.SOC_OCV_table_size - 1.0;
    }

    P_new_tmp = rtU.params.SOC_table[(int32_T)rtDW.soc_m - 1];
    P_new_tmp_0 = rtU.params.SOC_table[(int32_T)(rtDW.soc_m + 1.0) - 1] - P_new_tmp;
    if (P_new_tmp_0 == 0.0) {
      rtDW.tau = rtU.params.OCV_table[(int32_T)rtDW.soc_m - 1];
    } else {
      rtDW.b_idx = rtU.params.OCV_table[(int32_T)rtDW.soc_m - 1];
      rtDW.tau = ((1.0 - rtDW.rtb_x_new_idx_0) - P_new_tmp) / P_new_tmp_0 * (rtU.params.OCV_table[(int32_T)(rtDW.soc_m + 1.0) - 1] - rtDW.b_idx) + rtDW.b_idx;
    }
  }

  rtDW.tau = (rtU.params.N_series * rtDW.tau - rtb_x_new_idx_1) - rtU.params.R0_pack * rtU.u_k;
  rtDW.soc_m = rtU.z_k - rtDW.tau;
  rtDW.b_idx = H_idx_0 * rtDW.P_pred[0];
  rtDW.idx = ((rtDW.b_idx - rtDW.P_pred[1]) * H_idx_0 - (H_idx_0 * rtDW.P_pred[2] - rtDW.P_pred[3])) + rtU.params.R_meas;
  rtDW.b_idx = (rtDW.b_idx - rtDW.P_pred[2]) / rtDW.idx;
  kalman_gain_idx_0 = rtDW.b_idx;
  rtDW.rtb_x_new_idx_0 += rtDW.b_idx * rtDW.soc_m;
  rtDW.b_idx = (rtDW.P_pred[1] * H_idx_0 - rtDW.P_pred[3]) / rtDW.idx;

  /* Outport: '<Root>/x_new' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.x_new[1] = rtDW.b_idx * rtDW.soc_m + rtb_x_new_idx_1;

  /* MATLAB Function: '<Root>/soc_ekf_matlab' incorporates:
   *  Inport: '<Root>/params'
   *  Inport: '<Root>/z_k'
   *  Outport: '<Root>/x0'
   */
  Fd[1] = 0;
  Fd[2] = 0;
  Fd[0] = 1;
  Fd[3] = 1;
  rtDW.soc_m = rtU.z_k / rtU.params.N_series;
  if (rtDW.soc_m >= rtU.params.OCV_table[0]) {
    rtDW.soc_m = 1.0 - rtU.params.SOC_table[0];
  } else if (rtDW.soc_m <= rtU.params.OCV_table[99]) {
    rtDW.soc_m = 1.0 - rtU.params.SOC_table[99];
  } else {
    d_idx = 0;
    i = 0;
    while (i < 99) {
      if ((rtDW.soc_m <= rtU.params.OCV_table[i]) && (rtDW.soc_m >= rtU.params.OCV_table[i + 1])) {
        d_idx = i;
        break;
      } else {
        i++;
      }
    }

    P_new_tmp = rtU.params.OCV_table[d_idx + 1] - rtU.params.OCV_table[d_idx];
    if (P_new_tmp == 0.0) {
      rtDW.soc_m = 1.0 - rtU.params.SOC_table[d_idx];
    } else {
      rtDW.soc_m = 1.0 - ((rtDW.soc_m - rtU.params.OCV_table[d_idx]) / P_new_tmp * (rtU.params.SOC_table[d_idx + 1] - rtU.params.SOC_table[d_idx]) + rtU.params.SOC_table[d_idx]);
    }
  }

  rtY.x0[0] = fmin(fmax(rtDW.soc_m, 0.0), 1.0);
  rtY.x0[1] = 0.0;
  rtDW.P_new_tmp[0] = kalman_gain_idx_0 * H_idx_0;
  rtDW.P_new_tmp[1] = rtDW.b_idx * H_idx_0;
  rtDW.P_new_tmp[2] = -kalman_gain_idx_0;
  rtDW.P_new_tmp[3] = -rtDW.b_idx;
  rtb_x_new_idx_1 = 1.0 - rtDW.P_new_tmp[0];
  rtDW.soc_m = 0.0 - rtDW.P_new_tmp[1];
  for (d_idx = 0; d_idx < 2; d_idx++) {
    i = d_idx << 1;
    P_new_tmp = rtDW.P_pred[i];
    rtDW.Fd_m[i] = (real_T)Fd[d_idx] - rtDW.P_new_tmp[d_idx];
    rtDW.Fd_c[i] = 0.0;
    P_new_tmp_0 = rtDW.P_pred[i + 1];
    rtDW.Fd[i] = (0.0 - (-kalman_gain_idx_0)) * P_new_tmp_0 + rtb_x_new_idx_1 * P_new_tmp;
    rtDW.Fd[i + 1] = (1.0 - (-rtDW.b_idx)) * P_new_tmp_0 + rtDW.soc_m * P_new_tmp;
    rtDW.Fd_m[i + 1] = (real_T)Fd[d_idx + 2] - rtDW.P_new_tmp[d_idx + 2];
    rtDW.Fd_c[i + 1] = 0.0;
  }

  rtDW.soc_m = rtDW.Fd[0];
  rtDW.idx = rtDW.Fd[1];
  rtb_x_new_idx_1 = rtDW.Fd[2];
  H_idx_0 = rtDW.Fd[3];
  for (d_idx = 0; d_idx < 2; d_idx++) {
    i = d_idx << 1;
    P_new_tmp = rtDW.Fd_m[i];
    Fd_0 = rtDW.Fd_c[i + 1] + rtDW.idx * P_new_tmp;
    P_new_tmp_0 = rtDW.Fd_m[i + 1];
    rtDW.Fd_c[i] = (rtDW.soc_m * P_new_tmp + rtDW.Fd_c[i]) + rtb_x_new_idx_1 * P_new_tmp_0;
    rtDW.Fd_c[i + 1] = H_idx_0 * P_new_tmp_0 + Fd_0;
  }

  P_new_tmp = kalman_gain_idx_0 * rtU.params.R_meas;

  /* Outport: '<Root>/P_new' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.P_new[0] = P_new_tmp * kalman_gain_idx_0 + rtDW.Fd_c[0];

  /* MATLAB Function: '<Root>/soc_ekf_matlab' incorporates:
   *  Inport: '<Root>/params'
   */
  P_new_tmp_0 = rtDW.b_idx * rtU.params.R_meas;

  /* Outport: '<Root>/P_new' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.P_new[1] = P_new_tmp_0 * kalman_gain_idx_0 + rtDW.Fd_c[1];
  rtY.P_new[2] = P_new_tmp * rtDW.b_idx + rtDW.Fd_c[2];
  rtY.P_new[3] = P_new_tmp_0 * rtDW.b_idx + rtDW.Fd_c[3];

  /* Outport: '<Root>/x_new' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.x_new[0] = fmin(fmax(rtDW.rtb_x_new_idx_0, 0.0), 1.0);

  /* Outport: '<Root>/y_pred' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.y_pred = rtDW.tau;
}

/* Model initialize function */
void soc_ekf_matlab_initialize(void) {
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
