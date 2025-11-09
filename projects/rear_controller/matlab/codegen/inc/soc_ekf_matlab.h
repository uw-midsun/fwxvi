#pragma once

/************************************************************************************************
 * @file    soc_ekf_matlab.h
 *
 * @brief   Soc Ekf Matlab
 *
 * @date    2025-10-31
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "rtwtypes.h"

/* Intra-component Headers */

/**
 * @defgroup soc_ekf_matlab
 * @brief    soc_ekf_matlab Firmware
 * @{
 */

/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: soc_ekf_matlab.h
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

#ifndef RTW_HEADER_soc_ekf_matlab_h_
#define RTW_HEADER_soc_ekf_matlab_h_
#ifndef soc_ekf_matlab_COMMON_INCLUDES_
#define soc_ekf_matlab_COMMON_INCLUDES_
#endif /* soc_ekf_matlab_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm) ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val) ((rtm)->errorStatus = (val))
#endif

#define soc_ekf_matlab_M (rtM)

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

#ifndef DEFINED_TYPEDEF_FOR_battery_bus_
#define DEFINED_TYPEDEF_FOR_battery_bus_

typedef struct {
  real_T N_series;
  real_T N_parallel;
  real_T Q_cell_Ah;
  real_T R0_cell;
  real_T R1_cell;
  real_T C1_cell;
  real_T Q_pack_Ah;
  real_T R0_pack;
  real_T R1_pack;
  real_T C1_pack;
  real_T Q_proc[4];
  real_T R_meas;
  real_T rest_thresh;
  int32_T rest_window;
  real_T alpha;
  real_T SOC_OCV_table_size;
  real_T SOC_table[100];
  real_T OCV_table[100];
} battery_bus;

#endif

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  real_T Fd[4];
  real_T P_pred[4];
  real_T P_new_tmp[4];
  real_T Fd_m[4];
  real_T Fd_c[4];
  real_T kalman_gain[2];
  real_T tau;
  real_T soc_m;
} DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T x_prev[2];   /* '<Root>/x_prev' */
  real_T P_prev[4];   /* '<Root>/P_prev' */
  real_T u_k;         /* '<Root>/u_k' */
  real_T z_k;         /* '<Root>/z_k' */
  battery_bus params; /* '<Root>/params' */
  real_T dt;          /* '<Root>/dt' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T x_new[2]; /* '<Root>/x_new' */
  real_T P_new[4]; /* '<Root>/P_new' */
  real_T y_pred;   /* '<Root>/y_pred' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T *volatile errorStatus;
};

/* Block signals and states (default storage) */
extern DW rtDW;

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/* Model entry point functions */
extern void soc_ekf_matlab_initialize(void);
extern void soc_ekf_matlab_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('ekf_soc_estimator/soc_ekf_matlab')    - opens subsystem ekf_soc_estimator/soc_ekf_matlab
 * hilite_system('ekf_soc_estimator/soc_ekf_matlab/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'ekf_soc_estimator'
 * '<S1>'   : 'ekf_soc_estimator/soc_ekf_matlab'
 */
#endif /* RTW_HEADER_soc_ekf_matlab_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */

/** @} */
