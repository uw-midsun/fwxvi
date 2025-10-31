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
 * Model version                  : 1.43
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Thu Oct 30 18:39:26 2025
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
  real_T P_prev;
  real_T P_prev_0;
  real_T P_prev_1;
  real_T b_idx;
  real_T idx;
  real_T rtb_x_new_idx_0;
  real_T rtb_x_new_idx_1;
  int32_T Fd;
  int32_T k;
  boolean_T exitg1;

  /* MATLAB Function: '<Root>/soc_ekf_matlab' incorporates:
   *  Inport: '<Root>/P_prev'
   *  Inport: '<Root>/dt'
   *  Inport: '<Root>/params'
   *  Inport: '<Root>/u_k'
   *  Inport: '<Root>/x_prev'
   *  Inport: '<Root>/z_k'
   */
  idx = rtU.params.R1_pack * rtU.params.C1_pack;
  if (idx <= 0.0) {
    rtDW.tau = 0.0;
  } else {
    rtDW.tau = exp(-rtU.dt / idx);
  }

  rtb_x_new_idx_0 = fmin(fmax(rtU.u_k * rtU.dt / (rtU.params.Q_pack_Ah * 3600.0) + rtU.x_prev[0], 0.0), 1.0);
  rtb_x_new_idx_1 = (1.0 - rtDW.tau) * rtU.params.R1_pack * rtU.u_k + rtDW.tau * rtU.x_prev[1];
  if (idx <= 0.0) {
    rtDW.tau = 0.0;
  } else {
    rtDW.tau = exp(-rtU.dt / idx);
  }

  rtDW.Fd[0] = 1.0;
  rtDW.Fd[2] = 0.0;
  rtDW.Fd[1] = 0.0;
  rtDW.Fd[3] = rtDW.tau;
  P_prev = rtU.P_prev[1];
  P_prev_0 = rtU.P_prev[0];
  b_idx = rtU.P_prev[3];
  P_prev_1 = rtU.P_prev[2];
  for (k = 0; k < 2; k++) {
    rtDW.soc_m = rtDW.Fd[k + 2];
    Fd = (int32_T)rtDW.Fd[k];
    idx = rtDW.soc_m * P_prev + (real_T)Fd * P_prev_0;
    rtDW.soc_m = rtDW.soc_m * b_idx + (real_T)Fd * P_prev_1;
    rtDW.P_pred[k] = (rtDW.soc_m * 0.0 + idx) + rtU.params.Q_proc[k];
    rtDW.P_pred[k + 2] = (rtDW.soc_m * rtDW.tau + idx * 0.0) + rtU.params.Q_proc[k + 2];
  }

  rtDW.tau = fmin(rtb_x_new_idx_0 + 1.0E-5, 1.0);
  rtDW.soc_m = fmax(rtb_x_new_idx_0 - 1.0E-5, 0.0);
  if (1.0 - rtDW.tau <= rtU.params.SOC_table[0]) {
    idx = rtU.params.OCV_table[0];
  } else if (1.0 - rtDW.tau >= rtU.params.SOC_table[(int32_T)rtU.params.SOC_OCV_table_size - 1]) {
    idx = rtU.params.OCV_table[(int32_T)rtU.params.SOC_OCV_table_size - 1];
  } else {
    idx = -1.0;
    k = 0;
    exitg1 = false;
    while ((!exitg1) && (k <= (int32_T)(rtU.params.SOC_OCV_table_size - 1.0) - 1)) {
      if ((1.0 - rtDW.tau >= rtU.params.SOC_table[k]) && (1.0 - rtDW.tau <= rtU.params.SOC_table[k + 1])) {
        idx = (real_T)k + 1.0;
        exitg1 = true;
      } else {
        k++;
      }
    }

    if (idx == -1.0) {
      idx = rtU.params.SOC_OCV_table_size - 1.0;
    }

    P_prev = rtU.params.SOC_table[(int32_T)idx - 1];
    P_prev_0 = rtU.params.SOC_table[(int32_T)(idx + 1.0) - 1] - P_prev;
    if (P_prev_0 == 0.0) {
      idx = rtU.params.OCV_table[(int32_T)idx - 1];
    } else {
      P_prev_1 = rtU.params.OCV_table[(int32_T)idx - 1];
      idx = ((1.0 - rtDW.tau) - P_prev) / P_prev_0 * (rtU.params.OCV_table[(int32_T)(idx + 1.0) - 1] - P_prev_1) + P_prev_1;
    }
  }

  if (1.0 - rtDW.soc_m <= rtU.params.SOC_table[0]) {
    b_idx = rtU.params.OCV_table[0];
  } else if (1.0 - rtDW.soc_m >= rtU.params.SOC_table[(int32_T)rtU.params.SOC_OCV_table_size - 1]) {
    b_idx = rtU.params.OCV_table[(int32_T)rtU.params.SOC_OCV_table_size - 1];
  } else {
    b_idx = -1.0;
    k = 0;
    exitg1 = false;
    while ((!exitg1) && (k <= (int32_T)(rtU.params.SOC_OCV_table_size - 1.0) - 1)) {
      if ((1.0 - rtDW.soc_m >= rtU.params.SOC_table[k]) && (1.0 - rtDW.soc_m <= rtU.params.SOC_table[k + 1])) {
        b_idx = (real_T)k + 1.0;
        exitg1 = true;
      } else {
        k++;
      }
    }

    if (b_idx == -1.0) {
      b_idx = rtU.params.SOC_OCV_table_size - 1.0;
    }

    P_prev = rtU.params.SOC_table[(int32_T)b_idx - 1];
    P_prev_0 = rtU.params.SOC_table[(int32_T)(b_idx + 1.0) - 1] - P_prev;
    if (P_prev_0 == 0.0) {
      b_idx = rtU.params.OCV_table[(int32_T)b_idx - 1];
    } else {
      P_prev_1 = rtU.params.OCV_table[(int32_T)b_idx - 1];
      b_idx = ((1.0 - rtDW.soc_m) - P_prev) / P_prev_0 * (rtU.params.OCV_table[(int32_T)(b_idx + 1.0) - 1] - P_prev_1) + P_prev_1;
    }
  }

  b_idx = (rtU.params.N_series * idx - rtU.params.N_series * b_idx) / ((rtDW.tau - rtDW.soc_m) + 1.0E-12);
  if (1.0 - rtb_x_new_idx_0 <= rtU.params.SOC_table[0]) {
    rtDW.tau = rtU.params.OCV_table[0];
  } else if (1.0 - rtb_x_new_idx_0 >= rtU.params.SOC_table[(int32_T)rtU.params.SOC_OCV_table_size - 1]) {
    rtDW.tau = rtU.params.OCV_table[(int32_T)rtU.params.SOC_OCV_table_size - 1];
  } else {
    rtDW.soc_m = -1.0;
    k = 0;
    exitg1 = false;
    while ((!exitg1) && (k <= (int32_T)(rtU.params.SOC_OCV_table_size - 1.0) - 1)) {
      if ((1.0 - rtb_x_new_idx_0 >= rtU.params.SOC_table[k]) && (1.0 - rtb_x_new_idx_0 <= rtU.params.SOC_table[k + 1])) {
        rtDW.soc_m = (real_T)k + 1.0;
        exitg1 = true;
      } else {
        k++;
      }
    }

    if (rtDW.soc_m == -1.0) {
      rtDW.soc_m = rtU.params.SOC_OCV_table_size - 1.0;
    }

    P_prev = rtU.params.SOC_table[(int32_T)rtDW.soc_m - 1];
    P_prev_0 = rtU.params.SOC_table[(int32_T)(rtDW.soc_m + 1.0) - 1] - P_prev;
    if (P_prev_0 == 0.0) {
      rtDW.tau = rtU.params.OCV_table[(int32_T)rtDW.soc_m - 1];
    } else {
      idx = rtU.params.OCV_table[(int32_T)rtDW.soc_m - 1];
      rtDW.tau = ((1.0 - rtb_x_new_idx_0) - P_prev) / P_prev_0 * (rtU.params.OCV_table[(int32_T)(rtDW.soc_m + 1.0) - 1] - idx) + idx;
    }
  }

  rtDW.tau = (rtU.params.N_series * rtDW.tau - rtb_x_new_idx_1) - rtU.params.R0_pack * rtU.u_k;
  rtDW.soc_m = rtU.z_k - rtDW.tau;
  P_prev_1 = b_idx * rtDW.P_pred[0];
  idx = ((P_prev_1 - rtDW.P_pred[1]) * b_idx - (b_idx * rtDW.P_pred[2] - rtDW.P_pred[3])) + rtU.params.R_meas;
  P_prev = (P_prev_1 - rtDW.P_pred[2]) / idx;
  rtDW.kalman_gain[0] = P_prev;
  rtb_x_new_idx_0 += P_prev * rtDW.soc_m;
  P_prev = (rtDW.P_pred[1] * b_idx - rtDW.P_pred[3]) / idx;
  rtDW.kalman_gain[1] = P_prev;

  /* Outport: '<Root>/x_new' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.x_new[1] = P_prev * rtDW.soc_m + rtb_x_new_idx_1;

  /* MATLAB Function: '<Root>/soc_ekf_matlab' incorporates:
   *  Inport: '<Root>/params'
   */
  rtDW.Fd[1] = 0.0;
  rtDW.Fd[2] = 0.0;
  rtDW.Fd[0] = 1.0;
  rtDW.P_new_tmp[0] = rtDW.kalman_gain[0] * b_idx;
  rtDW.P_new_tmp[1] = P_prev * b_idx;
  rtDW.Fd[3] = 1.0;
  rtDW.P_new_tmp[2] = -rtDW.kalman_gain[0];
  rtDW.P_new_tmp[3] = -P_prev;
  rtDW.Fd_m[0] = 1.0 - rtDW.P_new_tmp[0];
  rtDW.Fd_m[1] = 0.0 - rtDW.P_new_tmp[1];
  rtDW.Fd_m[2] = 0.0 - (-rtDW.kalman_gain[0]);
  rtDW.Fd_m[3] = 1.0 - (-P_prev);
  rtb_x_new_idx_1 = rtDW.P_pred[1];
  P_prev_0 = rtDW.P_pred[0];
  b_idx = rtDW.P_pred[3];
  P_prev_1 = rtDW.P_pred[2];
  for (k = 0; k < 2; k++) {
    rtDW.soc_m = rtDW.Fd_m[k + 2];
    idx = rtDW.Fd_m[k];
    rtDW.P_pred[k] = rtDW.soc_m * rtb_x_new_idx_1 + idx * P_prev_0;
    Fd = k << 1;
    rtDW.Fd_c[Fd] = rtDW.Fd[k] - rtDW.P_new_tmp[k];
    rtDW.P_pred[k + 2] = rtDW.soc_m * b_idx + idx * P_prev_1;
    rtDW.Fd_c[Fd + 1] = rtDW.Fd[k + 2] - rtDW.P_new_tmp[k + 2];
  }

  rtDW.soc_m = rtDW.Fd_c[1];
  idx = rtDW.Fd_c[0];
  rtb_x_new_idx_1 = rtDW.Fd_c[3];
  P_prev_0 = rtDW.Fd_c[2];
  for (k = 0; k < 2; k++) {
    b_idx = rtDW.P_pred[k + 2];
    P_prev_1 = rtDW.P_pred[k];
    rtDW.Fd[k] = b_idx * rtDW.soc_m + P_prev_1 * idx;
    Fd = k << 1;
    rtDW.P_new_tmp[Fd] = rtDW.kalman_gain[0] * rtU.params.R_meas * rtDW.kalman_gain[k];
    rtDW.Fd[k + 2] = b_idx * rtb_x_new_idx_1 + P_prev_1 * P_prev_0;
    rtDW.P_new_tmp[Fd + 1] = P_prev * rtU.params.R_meas * rtDW.kalman_gain[k];
  }

  /* Outport: '<Root>/P_new' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.P_new[0] = rtDW.Fd[0] + rtDW.P_new_tmp[0];
  rtY.P_new[1] = rtDW.Fd[1] + rtDW.P_new_tmp[1];
  rtY.P_new[2] = rtDW.Fd[2] + rtDW.P_new_tmp[2];
  rtY.P_new[3] = rtDW.Fd[3] + rtDW.P_new_tmp[3];

  /* Outport: '<Root>/x_new' incorporates:
   *  MATLAB Function: '<Root>/soc_ekf_matlab'
   */
  rtY.x_new[0] = fmin(fmax(rtb_x_new_idx_0, 0.0), 1.0);

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
