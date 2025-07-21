#pragma once

/************************************************************************************************
 * @file   ekf.h
 *
 * @brief   Header file for the Extended Kalman Filter for the BMI323
 *
 * @date   2025-3-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
#include <stdio.h>
#include <math.h>
/** @brief Size of x matrix, containing position, velocity and acceleration */
#define STATE_SIZE 9
/** @brief Number of measurement inputs: 3D position (x, y, z) and 3D acceleration (ax, ay, az) */
#define MEASUREMENT_SIZE 6 //(x, y, z), (ax, ay, az), (a, b, y)
/** @brief 1 second time interval between successive measurements */
#define DT 1



/** @brief Initial state matrix */
double x[STATE_SIZE]={0};
/** @brief State covariance matrix (Uncertainty in X) */
double P[STATE_SIZE][STATE_SIZE]={0};
/** @brief Control input vector: acceleration inputs (ax, ay, az). */
double U[MEASUREMENT_SIZE]={0};


/** 
 * @brief Process noise covariance matrix.
 * Represents uncertainty in the model, including noise in position, velocity, and orientation.
 */
double Q[STATE_SIZE][STATE_SIZE]={0}; //9x9 matrix w/covariance in position, velocity, and orientation angle noise


/** 
 * @brief Measurement noise covariance matrix.
 * Initialized to identity since measurement errors are assumed independent and equally uncertain.
 */
double R[MEASUREMENT_SIZE][MEASUREMENT_SIZE]=
{{1, 0, 0, 0, 0, 0},
{0, 1, 0, 0, 0, 0},
{0, 0, 1, 0, 0, 0},
{0, 0, 0, 1, 0, 0},
{0, 0, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 1}
};

/** 
 * @brief State transition matrix A.
 * Encodes the kinematic model: positions integrate velocity, velocities integrate acceleration.
 */
extern double A[STATE_SIZE][STATE_SIZE] = {
    {1, 0, 0, DT, 0, 0,  0,  0,  0},
    {0, 1, 0, 0, DT, 0,  0,  0,  0},
    {0, 0, 1, 0, 0, DT,  0,  0,  0},
    {0, 0, 0, 1, 0, 0,   0,  0,  0},
    {0, 0, 0, 0, 1, 0,   0,  0,  0},
    {0, 0, 0, 0, 0, 1,   0,  0,  0},
    {0, 0, 0, 0, 0, 0,   1, DT, 0},
    {0, 0, 0, 0, 0, 0,   0, 1, DT},
    {0, 0, 0, 0, 0, 0,   0, 0, 1}
};//iniitialzie with time derivatives
double B[MEASUREMENT_SIZE][MEASUREMENT_SIZE];


/**
 * @brief Multiply two STATE_SIZE x STATE_SIZE matrices
 * @param firstMatrix First operand
 * @param secondMatrix Second operand
 * @param result Result of the multiplication
 */
void multiply_matrices(double firstMatrix[][STATE_SIZE], double secondMatrix[][STATE_SIZE], double result[][STATE_SIZE]);

/**
 * @brief Compute the transpose of a STATE_SIZE x STATE_SIZE matrix
 * @param matrix Input matrix
 * @param transpose Output transpose
 */
void transponse_matrix(double matrix[][STATE_SIZE], double transpose[][STATE_SIZE]);

/**
 * @brief Initializes process noise covariance matrix Q
 */
void noise_covariance();

/**
 * @brief Predicts the next state using the motion model
 */
void predict_state();

/**
 * @brief Predicts previous state (x_{k-1|k-1}) — useful for historical trace
 */
void predict_prev_state();

/**
 * @brief Predicts the next covariance matrix (P_{k|k-1})
 */
void predict_covariance();

/**
 * @brief Updates the state based on new measurements
 */
void update_state();
