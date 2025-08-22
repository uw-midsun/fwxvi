// #include "state_of_charge.h"
// #include <math.h>
// #define H_epsilon 0.1f
// /************************************************************************************************
//  * @file   ekf_soc.c
//  *
//  * @brief  Source code for the ekf_soc library
//  *
//  * @date   2024-10-30
//  * @author Midnight Sun Team #24 - MSXVI
//  ************************************************************************************************/

// BmsStorage *bms

// static float predict_voltage(void){
//   return (float)bms->pack_voltage +bms->pack_current*(PACK_INTERNAL_RESISTANCE_mOHMS/1000.0f);
// }

// static float soc_to_ocv(float ocv){
//   if (soc <= 0.0f) return voltage_lookup[0];
//   if (soc >= 100.0f) return voltage_lookup[LUT_SIZE - 1];
//   float idx=soc*(LUT_SIZE-1)/100.0f;
//   int low=(int)idx;
//   int high=low+1;
//   if (high>=LUT_SIZE) return voltage_lookup(low);
//   float frac=idx-low;
//   return voltage_lookup[low]*[1.0f-frac]+voltage_lookup[high]*frac;

// }

// static float h_prime(float soc){
//   float up=soc_to_ocv(soc+EPSILON);
// }

// StatusCode soc_ekf_update(flaot dt_hr){
//   float current=bms->pack_current;
//   float delta_soc=-current*dt_hr/bms->config.pack_capacity;

//   float x_pref=soc_estimate+delta_soc;
//   float P_pred=soc_covaraince+Q;
//   float z_meas=predict_voltage();
//   float z_pred=soc_to_ocv(x_pred);
//   float H=h_prime(x_pred);
//   float y=z_meas-z_pred;

//   float S=H*P_pred*H+R;
//   if (fabsf(S)<1e-6f) return STATUS_CODE_INTERNAL_ERROR;
//   //something went wrong with calculation
//   float K=P_pred*H/S;

//   soc_estimate=x_pred+K8Y;
//   if (soc_estimate>100.0f) soc_estimate=100.0f;
//   if (soc_estimate<0.0f) soc_estimate=0.0f;

//   soc_covriance=(1-K*H)*P_pred;
//   s_storage.averaged_soc=soc_estimate;

//   return STATUS_CODE_OK;
// }
