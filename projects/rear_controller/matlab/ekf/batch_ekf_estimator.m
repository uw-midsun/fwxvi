% =========================================================================
% @file    batch_ekf_estimator.m
% @brief   Batch EKF loop using ekf_step and apply_soft_reset
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Runs an Extended Kalman Filter (EKF) across a full dataset, applying
% periodic soft resets if the system is at rest
% Q and R are initialized once from params and carried through the loop
%
% -------------------------------------------------------------------------
% INPUTS:
%   params : struct
%       EKF parameters (includes rest_thresh, rest_window, alpha, etc.)
%   u      : double [1xN]
%       Input/control sequence
%   z      : double [1xN]
%       Measurement sequence
%   dt     : double
%       Sampling time step
%
% OUTPUTS:
%   ekf_out : struct
%       .x_hat  - estimated states (2xN)
%       .P      - covariance history (2x2xN)
%       .y_pred - predicted measurements (1xN)
%       .innov  - innovations (1xN)
%       .Q_hist - process noise covariance history (2x2xN)
%       .R_hist - measurement noise variance history (1xN)
%
% =========================================================================

function ekf_out = batch_ekf_estimator(params, u, z, dt)
    % --- Initialization ---
    N = numel(u);
    x_hat = zeros(2,N,'single');
    P_hist = zeros(2,2,N,'single');
    y_pred = zeros(1,N,'single');
    innov = zeros(1,N,'single');
    Q_hist = zeros(2,2,N,'single');
    R_hist = zeros(1,N,'single');

    % Initial state estimate
    x_hat(:,1) = estimate_initial_state(params, z(1:min(5,end)), u(1:min(5,end)));
    P = eye(2,'single') * single(0.01);
    P_hist(:,:,1) = P;

    % Initial noise covariances (same defaults as ekf_step)
    if isfield(params,'Q_proc')
        Q = single(params.Q_proc);
    else
        Q = single([1e-6, 0; 0, 1e-5]);
    end

    if isfield(params,'R_meas')
        R = single(params.R_meas);
    else
        R = single(1e-3);
    end

    Q_hist(:,:,1) = Q;
    R_hist(1) = R;

    % Use the adaptive step only if params.use_aekf exists and is true
    use_aekf = isfield(params,'use_aekf') && params.use_aekf;

    rest_counter = int32(0);
    was_reset = false;

    % --- EKF Loop ---
    for k = 2:N
        if use_aekf
            [x_new, P_new, y_k, Q, R] = aekf_step(x_hat(:,k-1), P, u(k), z(k), params, dt, Q, R);
        else
            [x_new, P_new, y_k] = ekf_step(x_hat(:,k-1), P, u(k), z(k), params, dt);
        end

        x_hat(:,k) = x_new;
        P = P_new;
        P_hist(:,:,k) = P;
        Q_hist(:,:,k) = Q;
        R_hist(k) = R;
        y_pred(k) = y_k;
        innov(k) = z(k) - y_k;

        % Rest detection
        if abs(u(k)) < params.rest_thresh
            rest_counter = rest_counter + 1;
        else
            rest_counter = int32(0);
            was_reset = false;
        end

        % Apply soft reset if enough rest detected
        if rest_counter >= int32(params.rest_window) && ~was_reset
            V_avg = mean(z(max(1,k-9):k));
            [x_hat(:,k), P] = apply_soft_reset(x_hat(:,k), P, V_avg, params, params.alpha);
            was_reset = true;
        end
    end

    % --- Outputs ---
    ekf_out.x_hat = x_hat;
    ekf_out.P = P_hist;
    ekf_out.y_pred = y_pred;
    ekf_out.innov = innov;
    ekf_out.Q_hist = Q_hist;
    ekf_out.R_hist = R_hist;
end
