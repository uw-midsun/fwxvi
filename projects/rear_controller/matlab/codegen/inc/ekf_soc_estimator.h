/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: ekf_soc_estimator.h
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

#ifndef ekf_soc_estimator_h_
#define ekf_soc_estimator_h_
#ifndef ekf_soc_estimator_COMMON_INCLUDES_
#define ekf_soc_estimator_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                                 /* ekf_soc_estimator_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

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
  real_T Fd_k[4];
  real_T tau;
  real_T soc_m;
  real_T idx;
  real_T b_idx;
} DW;

/* Constant parameters (default storage) */
typedef struct {
  /* Computed Parameter: Constant_Value
   * Referenced by: '<Root>/Constant'
   */
  battery_bus Constant_Value;
} ConstP;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T x_prev[2];                    /* '<Root>/x_prev' */
  real_T P_prev[4];                    /* '<Root>/P_prev' */
  real_T u_k;                          /* '<Root>/u_k' */
  real_T z_k;                          /* '<Root>/z_k' */
  real_T dt;                           /* '<Root>/dt' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T x_new[2];                     /* '<Root>/x_new' */
  real_T P_new[4];                     /* '<Root>/P_new' */
  real_T y_pred;                       /* '<Root>/y_pred' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* Block signals and states (default storage) */
extern DW rtDW;

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/* Constant parameters (default storage) */
extern const ConstP rtConstP;

/* Model entry point functions */
extern void ekf_soc_estimator_initialize(void);
extern void ekf_soc_estimator_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<Root>/Scope' : Unused code path elimination
 * Block '<Root>/Scope1' : Unused code path elimination
 * Block '<Root>/Scope2' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'ekf_soc_estimator'
 * '<S1>'   : 'ekf_soc_estimator/MATLAB Function'
 */
#endif                                 /* ekf_soc_estimator_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
