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
#include "status.h"
/** @brief Size of x matrix, containing position, velocity and acceleration */
#define STATE_SIZE 9
/** @brief Number of measurement inputs: 3D position (x, y, z) and 3D acceleration (ax, ay, az) */
#define MEASUREMENT_SIZE 6 //(x, y, z), (ax, ay, az), (a, b, y)
/** @brief 1 second time interval between successive measurements */
#define DT 1



/** @brief Initial state matrix */
extern double x[STATE_SIZE];
/** @brief State covariance matrix (Uncertainty in X) */
extern double P[STATE_SIZE][STATE_SIZE];
/** @brief Control input vector: acceleration inputs (ax, ay, az). */
extern double U[MEASUREMENT_SIZE];


/** 
 * @brief Process noise covariance matrix.
 * Represents uncertainty in the model, including noise in position, velocity, and orientation.
 */
extern double Q[STATE_SIZE][STATE_SIZE]; //9x9 matrix w/covariance in position, velocity, and orientation angle noise


/** 
 * @brief Measurement noise covariance matrix.
 * Initialized to identity since measurement errors are assumed independent and equally uncertain.
 */
extern double R[MEASUREMENT_SIZE][MEASUREMENT_SIZE];

/** 
 * @brief State transition matrix A.
 * Encodes the kinematic model: positions integrate velocity, velocities integrate acceleration.
 */
extern double A[STATE_SIZE][STATE_SIZE];//iniitialzie with time derivatives

double B[MEASUREMENT_SIZE][MEASUREMENT_SIZE];


/**
 * @brief Multiply two STATE_SIZE x STATE_SIZE matrices
 * @param firstMatrix First operand
 * @param secondMatrix Second operand
 * @param result Result of the multiplication
 */
StatusCode matrix_mult(int rows_A, int cols_A, int rows_B,int cols_B, double A[rows_A][cols_A], double B[rows_B][cols_B], double result[rows_A][cols_B]);

/**
 * @brief Compute the transpose of a STATE_SIZE x STATE_SIZE matrix
 * @param matrix Input matrix
 * @param transpose Output transpose
 */
StatusCode transpose_matrix(int rows, int cols, double matrix[rows][cols], double transpose[cols][rows]);


/**
 * @brief Take a matrix and outputs its inverse
 * @param matrix Input Matrix
 * @param Inverse Matrix Inverse
 *  */ 
StatusCode inverse_matrix(int size, double input[size][size], double output[size][size]);
/**
 * @brief Initializes process noise covariance matrix Q
 */
StatusCode noise_covariance();

/**
 * @brief Predicts the next state using the motion model
 */
StatusCode predict_state();

/**
 * @brief Predicts previous state (x_{k-1|k-1}) — useful for historical trace
 */
StatusCode predict_prev_state();

/**
 * @brief Predicts the next covariance matrix (P_{k|k-1})
 */
StatusCode predict_covariance();

/**
 * @brief Updates the state based on new measurements
 */
StatusCode update_state();
