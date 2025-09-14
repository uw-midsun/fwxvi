/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: ekf_soc_estimator.c
 *
 * Code generated for Simulink model 'ekf_soc_estimator'.
 *
 * Model version                  : 1.42
 * Simulink Coder version         : 25.1 (R2025a) 21-Nov-2024
 * C/C++ source code generated on : Sun Sep 14 17:54:27 2025
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "ekf_soc_estimator.h"
#include <math.h>
#include "rtwtypes.h"

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
void ekf_soc_estimator_step(void)
{
  real_T Fd;
  real_T Fd_0;
  real_T Fd_idx_1;
  real_T H;
  real_T H_idx_0;
  real_T rtb_x_new_idx_0;
  real_T rtb_x_new_idx_1;
  int32_T k;
  int32_T tmp;
  boolean_T exitg1;

  /* MATLAB Function: '<Root>/MATLAB Function' incorporates:
   *  Constant: '<Root>/Constant'
   *  Inport: '<Root>/P_prev'
   *  Inport: '<Root>/dt'
   *  Inport: '<Root>/u_k'
   *  Inport: '<Root>/x_prev'
   *  Inport: '<Root>/z_k'
   */
  rtDW.tau = exp(-rtU.dt / 100.0);
  rtb_x_new_idx_0 = fmin(fmax(rtU.u_k * rtU.dt / 145800.0 + rtU.x_prev[0], 0.0),
    1.0);
  rtb_x_new_idx_1 = (1.0 - rtDW.tau) * 0.2 * rtU.u_k + rtDW.tau * rtU.x_prev[1];
  rtDW.Fd[0] = 0.0;
  rtDW.Fd[1] = 0.0;
  rtDW.Fd[2] = 0.0;
  rtDW.Fd[3] = 0.0;
  for (k = 0; k < 2; k++) {
    tmp = k << 1;
    H = rtU.P_prev[tmp];
    rtDW.soc_m = rtDW.Fd[tmp] + H;
    H_idx_0 = rtDW.Fd[tmp + 1] + 0.0 * H;
    H = rtU.P_prev[tmp + 1];
    rtDW.Fd[tmp] = 0.0 * H + rtDW.soc_m;
    rtDW.Fd[tmp + 1] = rtDW.tau * H + H_idx_0;
  }

  for (k = 0; k < 2; k++) {
    rtDW.soc_m = rtDW.Fd[k + 2];
    H_idx_0 = rtDW.Fd[k];
    rtDW.P_pred[k] = (rtDW.soc_m * 0.0 + H_idx_0) +
      rtConstP.Constant_Value.Q_proc[k];
    rtDW.P_pred[k + 2] = (rtDW.soc_m * rtDW.tau + H_idx_0 * 0.0) +
      rtConstP.Constant_Value.Q_proc[k + 2];
  }

  rtDW.tau = fmin(rtb_x_new_idx_0 + 1.0E-5, 1.0);
  rtDW.soc_m = fmax(rtb_x_new_idx_0 - 1.0E-5, 0.0);
  if (1.0 - rtDW.tau <= 2.5263741715914674E-17) {
    rtDW.idx = 4.1903091620110029;
  } else {
    rtDW.idx = -1.0;
    k = 0;
    exitg1 = false;
    while ((!exitg1) && (k <= 98)) {
      if ((1.0 - rtDW.tau >= rtConstP.Constant_Value.SOC_table[k]) && (1.0 -
           rtDW.tau <= rtConstP.Constant_Value.SOC_table[k + 1])) {
        rtDW.idx = (real_T)k + 1.0;
        exitg1 = true;
      } else {
        k++;
      }
    }

    if (rtDW.idx == -1.0) {
      rtDW.idx = 99.0;
    }

    k = (int32_T)rtDW.idx - 1;
    H = rtConstP.Constant_Value.SOC_table[k];
    tmp = (int32_T)(rtDW.idx + 1.0) - 1;
    rtDW.b_idx = rtConstP.Constant_Value.SOC_table[tmp] - H;
    if (rtDW.b_idx == 0.0) {
      rtDW.idx = rtConstP.Constant_Value.OCV_table[k];
    } else {
      rtDW.idx = rtConstP.Constant_Value.OCV_table[k];
      rtDW.idx += ((1.0 - rtDW.tau) - H) / rtDW.b_idx *
        (rtConstP.Constant_Value.OCV_table[tmp] - rtDW.idx);
    }
  }

  rtDW.b_idx = -1.0;
  k = 0;
  exitg1 = false;
  while ((!exitg1) && (k <= 98)) {
    if ((1.0 - rtDW.soc_m >= rtConstP.Constant_Value.SOC_table[k]) && (1.0 -
         rtDW.soc_m <= rtConstP.Constant_Value.SOC_table[k + 1])) {
      rtDW.b_idx = (real_T)k + 1.0;
      exitg1 = true;
    } else {
      k++;
    }
  }

  if (rtDW.b_idx == -1.0) {
    rtDW.b_idx = 99.0;
  }

  k = (int32_T)rtDW.b_idx - 1;
  H = rtConstP.Constant_Value.SOC_table[k];
  tmp = (int32_T)(rtDW.b_idx + 1.0) - 1;
  rtDW.b_idx = rtConstP.Constant_Value.SOC_table[tmp] - H;
  if (rtDW.b_idx == 0.0) {
    rtDW.b_idx = rtConstP.Constant_Value.OCV_table[k];
  } else {
    H_idx_0 = rtConstP.Constant_Value.OCV_table[k];
    rtDW.b_idx = ((1.0 - rtDW.soc_m) - H) / rtDW.b_idx *
      (rtConstP.Constant_Value.OCV_table[tmp] - H_idx_0) + H_idx_0;
  }

  H_idx_0 = (36.0 * rtDW.idx - 36.0 * rtDW.b_idx) / ((rtDW.tau - rtDW.soc_m) +
    1.0E-12);
  if (1.0 - rtb_x_new_idx_0 <= 2.5263741715914674E-17) {
    rtDW.tau = 4.1903091620110029;
  } else {
    rtDW.soc_m = -1.0;
    k = 0;
    exitg1 = false;
    while ((!exitg1) && (k <= 98)) {
      if ((1.0 - rtb_x_new_idx_0 >= rtConstP.Constant_Value.SOC_table[k]) &&
          (1.0 - rtb_x_new_idx_0 <= rtConstP.Constant_Value.SOC_table[k + 1])) {
        rtDW.soc_m = (real_T)k + 1.0;
        exitg1 = true;
      } else {
        k++;
      }
    }

    if (rtDW.soc_m == -1.0) {
      rtDW.soc_m = 99.0;
    }

    k = (int32_T)rtDW.soc_m - 1;
    H = rtConstP.Constant_Value.SOC_table[k];
    tmp = (int32_T)(rtDW.soc_m + 1.0) - 1;
    rtDW.b_idx = rtConstP.Constant_Value.SOC_table[tmp] - H;
    if (rtDW.b_idx == 0.0) {
      rtDW.tau = rtConstP.Constant_Value.OCV_table[k];
    } else {
      rtDW.tau = rtConstP.Constant_Value.OCV_table[k];
      rtDW.tau += ((1.0 - rtb_x_new_idx_0) - H) / rtDW.b_idx *
        (rtConstP.Constant_Value.OCV_table[tmp] - rtDW.tau);
    }
  }

  rtDW.tau = (36.0 * rtDW.tau - rtb_x_new_idx_1) - 0.06 * rtU.u_k;
  rtDW.soc_m = rtU.z_k - rtDW.tau;
  rtDW.b_idx = H_idx_0 * rtDW.P_pred[0];
  H = (rtDW.b_idx - rtDW.P_pred[1]) * H_idx_0 - (H_idx_0 * rtDW.P_pred[2] -
    rtDW.P_pred[3]);
  rtDW.b_idx = (rtDW.b_idx - rtDW.P_pred[2]) / (H + 0.0099999997764825821);
  rtDW.idx = rtDW.b_idx;
  rtb_x_new_idx_0 += rtDW.b_idx * rtDW.soc_m;
  rtDW.b_idx = (rtDW.P_pred[1] * H_idx_0 - rtDW.P_pred[3]) / (H +
    0.0099999997764825821);

  /* Outport: '<Root>/x_new' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function'
   */
  rtY.x_new[1] = rtDW.b_idx * rtDW.soc_m + rtb_x_new_idx_1;

  /* MATLAB Function: '<Root>/MATLAB Function' incorporates:
   *  Constant: '<Root>/Constant'
   */
  rtDW.Fd[1] = 0.0;
  rtDW.Fd[2] = 0.0;
  rtDW.Fd[0] = 1.0;
  rtDW.P_new_tmp[0] = rtDW.idx * H_idx_0;
  rtDW.P_new_tmp[1] = rtDW.b_idx * H_idx_0;
  rtDW.Fd[3] = 1.0;
  rtDW.P_new_tmp[2] = -rtDW.idx;
  rtDW.P_new_tmp[3] = -rtDW.b_idx;
  rtb_x_new_idx_1 = 1.0 - rtDW.P_new_tmp[0];
  Fd_idx_1 = 0.0 - rtDW.P_new_tmp[1];
  for (k = 0; k < 2; k++) {
    tmp = k << 1;
    H = rtDW.P_pred[tmp];
    rtDW.soc_m = rtb_x_new_idx_1 * H;
    H_idx_0 = Fd_idx_1 * H;
    rtDW.Fd_c[tmp] = rtDW.Fd[k] - rtDW.P_new_tmp[k];
    rtDW.Fd_k[tmp] = 0.0;
    H = rtDW.P_pred[tmp + 1];
    rtDW.Fd_m[tmp] = (0.0 - (-rtDW.idx)) * H + rtDW.soc_m;
    rtDW.Fd_m[tmp + 1] = (1.0 - (-rtDW.b_idx)) * H + H_idx_0;
    rtDW.Fd_c[tmp + 1] = rtDW.Fd[k + 2] - rtDW.P_new_tmp[k + 2];
    rtDW.Fd_k[tmp + 1] = 0.0;
  }

  rtDW.soc_m = rtDW.Fd_m[0];
  H_idx_0 = rtDW.Fd_m[1];
  rtb_x_new_idx_1 = rtDW.Fd_m[2];
  Fd_idx_1 = rtDW.Fd_m[3];
  for (k = 0; k < 2; k++) {
    tmp = k << 1;
    H = rtDW.Fd_c[tmp];
    Fd = rtDW.soc_m * H + rtDW.Fd_k[tmp];
    Fd_0 = rtDW.Fd_k[tmp + 1] + H_idx_0 * H;
    H = rtDW.Fd_c[tmp + 1];
    rtDW.Fd_k[tmp] = rtb_x_new_idx_1 * H + Fd;
    rtDW.Fd_k[tmp + 1] = Fd_idx_1 * H + Fd_0;
  }

  H = rtDW.idx * 0.0099999997764825821;

  /* Outport: '<Root>/P_new' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function'
   */
  rtY.P_new[0] = H * rtDW.idx + rtDW.Fd_k[0];

  /* MATLAB Function: '<Root>/MATLAB Function' incorporates:
   *  Constant: '<Root>/Constant'
   */
  rtb_x_new_idx_1 = rtDW.b_idx * 0.0099999997764825821;

  /* Outport: '<Root>/P_new' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function'
   */
  rtY.P_new[1] = rtb_x_new_idx_1 * rtDW.idx + rtDW.Fd_k[1];
  rtY.P_new[2] = H * rtDW.b_idx + rtDW.Fd_k[2];
  rtY.P_new[3] = rtb_x_new_idx_1 * rtDW.b_idx + rtDW.Fd_k[3];

  /* Outport: '<Root>/x_new' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function'
   */
  rtY.x_new[0] = fmin(fmax(rtb_x_new_idx_0, 0.0), 1.0);

  /* Outport: '<Root>/y_pred' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function'
   */
  rtY.y_pred = rtDW.tau;
}

/* Model initialize function */
void ekf_soc_estimator_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
