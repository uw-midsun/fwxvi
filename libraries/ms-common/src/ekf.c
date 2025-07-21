
/************************************************************************************************
 * @file   log.c
 *
 * @brief  Source code for the Extended Kalman Filter for the BMI323
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdlib.h>
#include <math.h>
/* Inter-component Headers */
#include "ekf.h"
#include "status.h"
/* Intra-component Headers */
#include "log.h"

/**
*@brief Maximum acceptable error
*/
#define EPSILON 1e-9

 /** @brief State transition matrix (Intialize diagonals with time derivatives of kinematic equations)*/ 
double A[STATE_SIZE][STATE_SIZE] = {
    {1, 0, 0, DT, 0, 0,  0,  0,  0},
    {0, 1, 0, 0, DT, 0,  0,  0,  0},
    {0, 0, 1, 0, 0, DT,  0,  0,  0},
    {0, 0, 0, 1, 0, 0,   0,  0,  0},
    {0, 0, 0, 0, 1, 0,   0,  0,  0},
    {0, 0, 0, 0, 0, 1,   0,  0,  0},
    {0, 0, 0, 0, 0, 0,   1, DT, 0},
    {0, 0, 0, 0, 0, 0,   0, 1, DT},
    {0, 0, 0, 0, 0, 0,   0, 0, 1}
};


/**
*@brief Takes in 2 matrices and performs matrix multiplication
*/
StatusCode matrix_mult(double A[][STATE_SIZE], double B[][STATE_SIZE], double result[][STATE_SIZE]) {
    if (!A||!B||!result) return STATUS_CODE_INVALID_ARGS;
    
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            result[i][j] = 0;
            for (int k = 0; k < STATE_SIZE; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return STATUS_CODE_OK
}

/**
*@brief Takes in matrix and outputs its tranpose
*/
StatusCode matrix_transpose(double matrix[][STATE_SIZE], double transpose[][STATE_SIZE]){
    if (!matrix||!transponse) return STATUS_CODE_INVALID_ARGS
    for (int i=0; i<STATE_SIZE; i++){
        for (int j=0; j<STATE_SIZE; j++){
            transpose[i][j]=matrix[j][i];
        }
    }
    return STATUS_CODE_OK
}


/**
*@brief Takes in matrix and outputs its inverse
 */
 StatusCode inverse_matrix(double input[MEASUREMENT_SIZE][MEASUREMENT_SIZE], 
                          double output[MEASUREMENT_SIZE][MEASUREMENT_SIZE]) {
                        int i, j, k;
                        
//Initalize output to identity matrix
    for (i=0; i<MEASUREMENT_SIZE; i++){
        for (j=0; j<MEASUREMENT_SIZE; j++){
            output[i][j]=(i==j) ? 1.0:0.0;
            }
    }
    //Create augmeneted matrix
    double temp[MEASUREMENT_SIZE][2*MEASUREMENT_SIZE];
    for (int i=0; i<MEASUREMENT_SIZE; i++){
        for (j=0; j<MEASUREMENT_SIZE; j++){
            temp[i][j]=input[i][j];
            temp[i][j+MEASUREMENT_SIZE]=output[i][j];
        }
    }
    //Gauss Jordan
    for (i=0; i<MEASUREMENT_SIZE; i++){
        double pivot=temp[i][i];
        //Matrix isnt invertible
        if (fabs(pivot)<EPSILON) return STATUS_CODE_INVALID_ARGS;

        for (j=0; j<2*MEASUREMENT_SIZE; j++){
            temp[i][j]/=pivot;
        }

        for (k=0; k<MEASUREMENT_SIZE; j++){
            if (k==i) continue;
            double factor=temp[k][i];
            for (j=0; j<2*MEASUREMENT_SIZE; j++){
                temp[k][j]-=factor*temp;
            }
        }
    }
    for (i=0; i<MEASUREMENT_SIZE; i++){
        for (j=0; j<MEASUREMENT_SIZE; j++){
            output[i][j]=temp[i][j+MEASUREMENT_SIZE];
        }
    }
    
    return STATUS_CODE_OK;

}

//use kinematics eqns to predict state
StatusCode predict_state(void){
    for (int i=0; i<3; i++){
        x[i]+=x[i+3]+0.5*U[i]*DT*DT;
        x[i+3]+=U[i]*DT;
        // Using equation X(t)=V(t)*DT
    }
    
    for (int i=6; i<STATE_SIZE; i++){
        x[i]+=U[i-3]*DT;
    }
    return STATUS_CODE_OK
}


//compute P K|K-1
StatusCode predict_covariance(void) {
    double A_T[STATE_SIZE][STATE_SIZE]={};
    double temp[STATE_SIZE][STATE_SIZE]={};
    double P_curr[STATE_SIZE][STATE_SIZE] = {0};

    matrix_transpose(A, A_T);
    matrix_mult(A, P, P_curr);
    matrix_mult(P_curr, A_T, P);

    // Add process noise Q
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] += Q[i][j];
        }
    }
    return STATUS_CODE_OK;
}

/**
*@brief Update state using measurement and kalman gain
*/
StatusCode update_state(void) {
    double H[MEASUREMENT_SIZE][STATE_SIZE] = {0}; // map state vector to measurements
    double K[STATE_SIZE][MEASUREMENT_SIZE] = {0}; //kalman gain
    double S[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};//uncertainty in residual
    double S_inv[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};
    double H_T[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    double P_H_T[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    double y[MEASUREMENT_SIZE] = {0};


    // initialize H matrix (maps state to measurement)
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        H[i][i + 3] = 1.0;
    }


    // y~ = z - H * x
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        y[i]=U[i];
        for (int j=0; i<STATE_SIZE; j++){
            y[i] -= H[i][j] * x[j];
        }
    }

    // compute S|k = H * P K|K-1 * H^T + R
    matrix_transpose(H, H_T);
    matrix_mult(H, P, P_H_T);
    matrix_mult(P_H_T, H_T, S);


    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            S[i][j] += R[i][j];
        }
    }


    inverse_matrix(S, S_inv);
    matrix_mult(P_H_T, S_inv, K);


    // Update state estimate: x = x + K * y
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            x[i] += K[i][j] * y[j];
        }
    }


    // Update covariance: P = (I - K*H) * P
    double I_KH[STATE_SIZE][STATE_SIZE] = {0};
    double KH[STATE_SIZE][STATE_SIZE] = {0};


    matrix_mult(K, H, KH);


    // create P K|K
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            I_KH[i][j] = (i == j ? 1 : 0) - KH[i][j];
            //create an identity matrix and then sutract the KH matrix
        }
    }


    double P_temp[STATE_SIZE][STATE_SIZE] = {0};
    matrix_mult(I_KH, P, P_temp);

    //Copy results back to P
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] = P_temp[i][j];
        }
    }
    return STATUS_CODE_OK;

}




