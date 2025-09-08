% =========================================================================
% @file    test_soc_ssm.m
% @brief   Test suite for the battery state-space model (SSM)
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Runs three validation scenarios for the state-space model:
%   1. Linear Test      - long discharge/charge sweep
%   2. HPPC Test        - hybrid pulse power characterization
%   3. Automotive Test  - random current profile resembling real drive cycles
%
% Each test simulates a "true" system, logs state/voltage evolution, and
% generates figures for verification.
%
% -------------------------------------------------------------------------
% DEPENDENCIES:
%   - ssm_state_update.m
%   - ssm_output_fn.m
%   - ssm_output_jacobian.m
%   - ssm_state_jacobian.m
%   - battery_params.m
%   - ocv_from_soc.m
%   - clamp01.m
%
% =========================================================================

function test_soc_ssm()
    addpath('..');
    addpath('../utils');

    fprintf('Running Linear Test...\n');
    test_battery_ssm_linear();
    
    fprintf('Running HPPC Test...\n');
    test_battery_ssm_hppc();
    
    fprintf('Running Automotive Test...\n');
    test_battery_ssm_automotive();
end

function test_battery_ssm_linear()
    % Full-range visual test of the battery state-space model

    %% Load model + params
    params = battery_params();
    dt = 1;                % Time step in seconds
    T_sim = 10000;         % Total sim time [s]
    time = 0:dt:T_sim;
    N = length(time);

    %% Long discharge and charge
    I = zeros(1, N);
    I(time < 3600) = -params.Q_pack_Ah;                      % Discharge for 1hr
    I(time >= 3600 & time < 7200) = (params.Q_pack_Ah / 2);  % Charge slowly for 1hr
    % rest is rest phase

    %% Initialize state
    x = zeros(2, N);
    y = zeros(1, N);
    ocv = zeros(1, N);
    grad = zeros(1, N);

    x(:,1) = [1.0; 0.0];   % Start fully charged
    y(1) = ssm_output_fn(x(:,1), I(1), params);
    ocv(1) = ocv_from_soc(params, x(1,1));
    H = ssm_output_jacobian(x(:,1), I(1), params);
    grad(1) = H(1);  % dOCV/dSOC

    %% Run simulation
    for k = 2:N
        x(:,k) = ssm_state_update(x(:,k-1), I(k-1), params, dt);
        y(k) = ssm_output_fn(x(:,k), I(k), params);
        ocv(k) = ocv_from_soc(params, x(1,k));
        H = ssm_output_jacobian(x(:,k), I(k), params);
        grad(k) = H(1);  % dOCV/dSOC
    end

    %% Plot results
    figure('Name','Battery SSM — Full Sweep','Position',[100 100 1600 800]);

    % --- SOC ---
    subplot(3,2,1);
    plot(time, x(1,:), 'b-', 'LineWidth', 2);
    ylabel('SOC'); xlabel('Time [s]');
    title('State of Charge Sweep'); grid on; ylim([-0.05 1.05]);

    % --- V_rc ---
    subplot(3,2,2);
    plot(time, x(2,:), 'm-', 'LineWidth', 2);
    ylabel('V_{RC} [V]'); xlabel('Time [s]');
    title('RC Branch Voltage'); grid on;

    % --- Terminal Voltage ---
    subplot(3,2,3);
    plot(time, y, 'r-', 'LineWidth', 2); hold on;
    plot(time, ocv, 'g--', 'LineWidth', 1.5);
    ylabel('Voltage [V]'); xlabel('Time [s]');
    legend('Terminal V', 'OCV');
    title('Battery Terminal Voltage vs OCV'); grid on;

    % --- OCV vs SOC curve (not time-based) ---
    subplot(3,2,4);
    plot(x(1,:), ocv, 'k-', 'LineWidth', 2);
    xlabel('SOC'); ylabel('OCV [V]');
    title('OCV vs SOC'); grid on; xlim([0 1]);

    % --- Current Profile ---
    subplot(3,2,5);
    plot(time, I, 'k-', 'LineWidth', 2);
    ylabel('Current [A]'); xlabel('Time [s]');
    title('Current Profile'); grid on;

    % --- dOCV/dSOC (sensitivity) ---
    subplot(3,2,6);
    plot(time, grad, 'c-', 'LineWidth', 2);
    ylabel('dOCV/dSOC'); xlabel('Time [s]');
    title('OCV Gradient'); grid on;

    %% Summary
    fprintf('\nFinal SOC: %.3f\n', x(1,end));
    fprintf('Final terminal voltage: %.3f V\n', y(end));
end

function test_battery_ssm_hppc()
    % HPPC-style pulse test of the battery state-space model

    %% Load model + params
    params = battery_params();
    dt = 1;                % Time step in seconds
    N_pulses = 20;
    pulse_len = 10;        % 10 seconds pulse
    rest_len  = 50;        % 50 seconds rest
    T_sim = (pulse_len + rest_len) * N_pulses;
    time = 0:dt:T_sim;
    N = length(time);

    %% Generate HPPC pulse profile
    I = zeros(1, N);
    amp = 3;  % Amplitude of current pulse

    for i = 0:N_pulses-1
        t_start = i * (pulse_len + rest_len);
        t_pulse = t_start + (1:pulse_len);
        if mod(i,2) == 0
            I(t_pulse + 1) = -amp;  % Discharge
        else
            I(t_pulse + 1) = amp;   % Charge
        end
    end

    %% Initialize state
    x = zeros(2, N);
    y = zeros(1, N);
    ocv = zeros(1, N);
    grad = zeros(1, N);

    x(:,1) = [0.7; 0.0];   % Start mid-SOC
    y(1) = ssm_output_fn(x(:,1), I(1), params);
    ocv(1) = ocv_from_soc(params, x(1,1));
    H = ssm_output_jacobian(x(:,1), I(1), params);
    grad(1) = H(1);  % dOCV/dSOC

    %% Simulate
    for k = 2:N
        x(:,k) = ssm_state_update(x(:,k-1), I(k-1), params, dt);
        y(k) = ssm_output_fn(x(:,k), I(k), params);
        ocv(k) = ocv_from_soc(params, x(1,k));
        H = ssm_output_jacobian(x(:,k), I(k), params);
        grad(k) = H(1);  % dOCV/dSOC
    end

    %% Plot
    figure('Name','Battery SSM — HPPC Test','Position',[100 100 1600 800]);

    subplot(3,2,1); plot(time, x(1,:), 'b', 'LineWidth', 2); 
    title('SOC'); ylabel('SOC'); xlabel('Time [s]'); grid on;

    subplot(3,2,2); plot(time, x(2,:), 'm', 'LineWidth', 2); 
    title('V_{RC}'); ylabel('V_{RC} [V]'); xlabel('Time [s]'); grid on;

    subplot(3,2,3); 
    plot(time, y, 'r', 'LineWidth', 2); hold on;
    plot(time, ocv, 'g--', 'LineWidth', 2); 
    title('Terminal Voltage vs OCV'); ylabel('Voltage [V]'); xlabel('Time [s]'); 
    legend('V_{term}', 'OCV'); grid on;

    subplot(3,2,4); plot(x(1,:), ocv, 'k-', 'LineWidth', 2); 
    title('OCV vs SOC'); xlabel('SOC'); ylabel('OCV [V]'); grid on;

    subplot(3,2,5); plot(time, I, 'k-', 'LineWidth', 2); 
    title('Current Profile (HPPC)'); ylabel('I [A]'); xlabel('Time [s]'); grid on;

    subplot(3,2,6); plot(time, grad, 'c-', 'LineWidth', 2); 
    title('dOCV/dSOC'); ylabel('Gradient'); xlabel('Time [s]'); grid on;

    %% Summary
    fprintf('\nFinal SOC: %.3f\n', x(1,end));
    fprintf('Final terminal voltage: %.3f V\n', y(end));
end

function test_battery_ssm_automotive()
    % Automotive-style random current profile test

    %% Load model + params
    params = battery_params();
    dt = 1;                  % Time step [s]
    T_sim = 1800;            % Simulate 30 minutes
    time = 0:dt:T_sim;
    N = length(time);

    %% Generate realistic automotive current profile
    rng(42); % for reproducibility
    I = zeros(1, N);

    for k = 1:N
        if mod(k, 30) == 0
            % Every ~30s, switch current
            I(k:end) = I(k:end) + (randn()*2.5);  % Add random offset
        end

        % Simulate regenerative braking (brief high charge)
        if mod(k, 300) == 0
            I(k:min(k+10,N)) = 3 + rand();  % Charge burst
        end

        % Simulate short idling
        if mod(k, 120) == 0
            I(k:min(k+10,N)) = 0;  % Idle phase
        end

        % Clamp current to realistic range [-5A, +5A]
        I(k) = max(min(I(k), 5), -5);
    end

    %% Initialize state
    x = zeros(2, N);
    y = zeros(1, N);
    ocv = zeros(1, N);
    grad = zeros(1, N);

    x(:,1) = [0.7; 0.0];  % Start from typical mid SOC
    y(1) = ssm_output_fn(x(:,1), I(1), params);
    ocv(1) = ocv_from_soc(params, x(1,1));
    H = ssm_output_jacobian(x(:,1), I(1), params);
    grad(1) = H(1);  % dOCV/dSOC

    %% Simulate
    for k = 2:N
        x(:,k) = ssm_state_update(x(:,k-1), I(k-1), params, dt);
        y(k) = ssm_output_fn(x(:,k), I(k), params);
        ocv(k) = ocv_from_soc(params, x(1,k));
        H = ssm_output_jacobian(x(:,k), I(k), params);
        grad(k) = H(1);  % dOCV/dSOC
    end

    %% Plot results
    figure('Name','Battery SSM — Automotive Test','Position',[100 100 1600 800]);

    subplot(3,2,1); plot(time, x(1,:), 'b-', 'LineWidth', 2);
    title('State of Charge (SOC)'); ylabel('SOC'); xlabel('Time [s]'); grid on; ylim([-0.05 1.05]);

    subplot(3,2,2); plot(time, x(2,:), 'm-', 'LineWidth', 2);
    title('RC Branch Voltage V_{RC}'); ylabel('V_{RC} [V]'); xlabel('Time [s]'); grid on;

    subplot(3,2,3);
    plot(time, y, 'r-', 'LineWidth', 2); hold on;
    plot(time, ocv, 'g--', 'LineWidth', 1.5);
    title('Terminal Voltage vs OCV'); ylabel('Voltage [V]'); xlabel('Time [s]');
    legend('Terminal Voltage', 'OCV'); grid on;

    subplot(3,2,4);
    plot(x(1,:), ocv, 'k-', 'LineWidth', 2);
    xlabel('SOC'); ylabel('OCV [V]');
    title('OCV vs SOC'); grid on; xlim([0 1]);

    subplot(3,2,5); plot(time, I, 'k-', 'LineWidth', 2);
    ylabel('Current [A]'); xlabel('Time [s]');
    title('Automotive-like Current Profile'); grid on;

    subplot(3,2,6); plot(time, grad, 'c-', 'LineWidth', 2);
    ylabel('dOCV/dSOC'); xlabel('Time [s]');
    title('OCV Sensitivity'); grid on;

    %% Summary
    fprintf('\nFinal SOC: %.3f\n', x(1,end));
    fprintf('Final terminal voltage: %.3f V\n', y(end));
end