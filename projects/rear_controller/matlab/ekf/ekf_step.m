% =========================================================================
% @file    ekf_step.m
% @brief   One Extended Kalman Filter iteration for battery SOC estimation
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Performs a single EKF cycle (predict + update) for a 2-state battery model
%
% -------------------------------------------------------------------------
% INPUTS:
%   x_prev  : double [2x1]
%       Previous state estimate [SOC; Vrc]
%   P_prev  : double [2x2]
%       Previous error covariance matrix
%   u_k     : double [scalar]
%       Current input (battery current, Amperes)
%   z_k     : double [scalar]
%       Current measurement (terminal voltage, Volts)
%   params  : struct
%       Battery model parameters
%   dt      : double [scalar]
%       Sampling time step (seconds)
%
% OUTPUTS:
%   x_new   : double [2x1]
%       Updated state estimate [SOC; Vrc]
%   P_new   : double [2x2]
%       Updated error covariance matrix
%   y_pred  : double [scalar]
%       Predicted measurement (terminal voltage, Volts)
%
% =========================================================================

function [x_new, P_new, y_pred] = ekf_step(x_prev, P_prev, u_k, z_k, params, dt)
    % =========================================================================
    % NOISE COVARIANCE SETUP
    %
    % Process noise Q: How much we trust our battery model
    % Measurement noise R: How much we trust our voltage sensor
    % =========================================================================
    
    if isfield(params,'Q_proc')
        Q = params.Q_proc;                         % Use provided process noise
    else
        Q = single([1e-6, 0; 0, 1e-5]);            % Default: SOC very certain, Vrc less certain
    end
    
    if isfield(params,'R_meas')
        R = params.R_meas;                         % Use provided measurement noise
    else
        R = single(1e-3);
    end

    % =========================================================================
    % PREDICT STEP
    %
    % Use the battery state space model to predict where we'll be next
    % =========================================================================
    
    % Predict the next state using our battery model
    x_pred = ssm_state_update(x_prev, u_k, params, dt);
    x_pred(1) = clamp01(x_pred(1));

    % Get the linearized state transition matrix (Jacobian)
    Fd = ssm_state_jacobian(x_prev, u_k, params, dt);
    
    % Predict how uncertain we are about our prediction
    P_pred = Fd * P_prev * Fd' + Q;

    % =========================================================================
    % UPDATE STEP  
    %
    % Use the voltage measurement to correct our prediction
    % =========================================================================
    
    % Get the measurement sensitivity matrix (how voltage depends on states)
    H = ssm_output_jacobian(x_pred, u_k, params); % [dV/dSOC, dV/dVrc] = [OCV_slope, -1]

    % Predict what voltage we should see given our state prediction
    y_pred = ssm_output_fn(x_pred, u_k, params);  % V_terminal = OCV(SOC) - R0*I - Vrc

    % Calculate the innovation (how wrong our prediction was)
    voltage_error = z_k - y_pred;                 % Actual voltage - predicted voltage
    
    % Calculate innovation covariance (total uncertainty in the measurement space)
    measurement_uncertainty  = H * P_pred * H' + R;
    
    % Calculate Kalman gain (how much to trust the measurement vs prediction)
    kalman_gain = (P_pred * H') / measurement_uncertainty;  % Higher gain = trust measurement more
    
    % Update state estimate using the measurement
    x_new = x_pred + kalman_gain * voltage_error; % Blend prediction with measurement
    x_new(1) = clamp01(x_new(1));                 % Keep SOC physically reasonable
    
    % Update uncertainty using Joseph form (numerically stable)
    I2 = eye(2,'like',P_pred);                    % 2x2 identity matrix
    P_new = (I2 - kalman_gain * H) * P_pred * (I2 - kalman_gain * H)' + kalman_gain * R * kalman_gain';
end
