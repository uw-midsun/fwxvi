#ifndef EKF_H
#define EKF_H

#include <stdio.h>
#include <math.h>

// Constants
#define STATE_SIZE 9
#define MEASUREMENT_SIZE 6  // (x, y, z), (ax, ay, az), (a, b, y)
#define DT 1

// State Variables
extern float x[STATE_SIZE];  // state matrix
extern float P[STATE_SIZE][STATE_SIZE];  // state covariance matrix
extern float U[MEASUREMENT_SIZE];  // input controls (ax, ay, az)

//  Matrices
extern float Q[STATE_SIZE][STATE_SIZE];  // process noise covariance matrix
extern float R[MEASUREMENT_SIZE][MEASUREMENT_SIZE];  // measurement noise covariance matrix
extern float A[STATE_SIZE][STATE_SIZE];  // state transition matrix
extern float B[MEASUREMENT_SIZE][MEASUREMENT_SIZE];

// matrix Operation Functions
void matrix_mult(int row1, int col1, float first[][col1],
                int row2, int col2, float second[][col2],
                float result[][col2]);

void matrix_transpose(float matrix[][STATE_SIZE],
                     float transpose[][STATE_SIZE]);

float determinant(float matrix[][STATE_SIZE], int n);

void cofactor(float matrix[][STATE_SIZE],
             float temp[][STATE_SIZE],
             int p, int q, int n);

void adjoint(float matrix[][STATE_SIZE],
            float adj[][STATE_SIZE],
            int n);

int inverse(float matrix[][STATE_SIZE],
           float inverse[][STATE_SIZE],
           int n);

// EKF Core Functions
void predict_state(float x[STATE_SIZE]);
void predict_covariance(void);
void update_state(float x[STATE_SIZE]);

#endif /* EKF_H */