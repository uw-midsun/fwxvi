% =========================================================================
% @file    test_batch_ekf_estimator.m
% @brief   Comprehensive test suite for batch EKF SOC estimation
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Runs a full suite of validation scenarios for the batch EKF implementation:
%
%   1. Full Sweep Test      - full-range SOC sweep from 100% to 0% and back
%   2. HPPC Test            - hybrid pulse power characterization
%   3. Automotive Test      - random dynamic current profile simulating driving
%   4. Multi-cycle Test     - repeated charge/discharge cycles
%   5. Long Discharge Test  - extended low-rate discharge for stress testing
%
% Each test simulates a "true" system, runs the EKF estimator, compares
% results, and plots SOC, voltage predictions, innovation, and estimation error.
%
% -------------------------------------------------------------------------
% DEPENDENCIES:
%   - batch_ekf_estimator.m
%   - ssm_state_update.m
%   - ssm_output_fn.m
%   - battery_params.m
%
% =========================================================================

function test_batch_ekf_estimator()
    addpath('..');
    addpath('../utils');
    addpath('../state_space_model');

    test_battery_ekf_full_sweep();
    test_battery_ekf_hppc();
    test_battery_ekf_automotive();
    test_battery_ekf_multicycle();
    test_battery_ekf_long_discharge_stress();
end

function test_battery_ekf_full_sweep()
    params = battery_params();
    dt = 1;
    T_sim = 14400;
    time = 0:dt:T_sim;
    N = length(time);

    I = zeros(1,N);
    I(time < 7200) = -(params.Q_pack_Ah / 2);
    I(time >=7200) = (params.Q_pack_Ah / 2);

    [x_true, y_true] = simulate_true(params, I, dt);
    y_noisy = y_true + 0.005*randn(size(y_true));

    ekf_out = batch_ekf_estimator(params, I, y_noisy, dt);
    plot_comprehensive_analysis(time, x_true, y_true, y_noisy, ekf_out, I, 'Full SOC Sweep');
end

function test_battery_ekf_hppc()
    params = battery_params();
    dt = 1;
    N_pulses = 30;
    pulse_len = 10;
    rest_len = 40;
    T_sim = (pulse_len + rest_len) * N_pulses;
    time = 0:dt:T_sim;
    N = length(time);

    I = zeros(1,N);
    amp = params.Q_cell_Ah;
    for i = 0:N_pulses-1
        t_start = i * (pulse_len + rest_len);
        pulse_idx = (t_start+1):(t_start+pulse_len);
        if mod(i,2) == 0
            I(pulse_idx) = -amp;
        else
            I(pulse_idx) = amp;
        end
    end

    [x_true, y_true] = simulate_true(params, I, dt);
    y_noisy = y_true + 0.015*randn(size(y_true));

    ekf_out = batch_ekf_estimator(params, I, y_noisy, dt);
    plot_comprehensive_analysis(time, x_true, y_true, y_noisy, ekf_out, I, 'HPPC Test');
end

function test_battery_ekf_automotive()
    params = battery_params();
    dt = 1;
    T_sim = 1800;
    time = 0:dt:T_sim;
    N = length(time);

    rng(42);
    I = zeros(1, N);
    for k = 1:N
        if mod(k, 25) == 0
            I(k:end) = I(k:end) + (randn() * 2);
        end
        if mod(k, 300) == 0
            I(k:min(k+10,N)) = params.Q_pack_Ah + rand();
        end
        if mod(k, 120) == 0
            I(k:min(k+10,N)) = 0;
        end
        I(k) = max(min(I(k), params.Q_pack_Ah), -params.Q_pack_Ah);
    end

    [x_true, y_true] = simulate_true(params, I, dt);
    y_noisy = y_true + 0.008*randn(size(y_true));

    ekf_out = batch_ekf_estimator(params, I, y_noisy, dt);
    plot_comprehensive_analysis(time, x_true, y_true, y_noisy, ekf_out, I, 'Automotive Cycle');
end

function test_battery_ekf_multicycle()
    params = battery_params();
    dt = 1;
    cycles = 5;
    T_cycle = 3600;

    I = repmat([-(params.Q_pack_Ah)*ones(1,T_cycle/2), params.Q_pack_Ah*ones(1,T_cycle/2)], 1, cycles);
    N = length(I);
    time = (0:N-1) * dt;

    [x_true, y_true] = simulate_true(params, I, dt);
    y_noisy = y_true + 0.01*randn(size(y_true));

    ekf_out = batch_ekf_estimator(params, I, y_noisy, dt);
    plot_comprehensive_analysis(time, x_true, y_true, y_noisy, ekf_out, I, 'Multi-cycle Charge/Discharge');
end

function test_battery_ekf_long_discharge_stress()
    params = battery_params();
    dt = 10;
    T_sim = 36000;
    time = 0:dt:T_sim;
    N = length(time);

    I = -(params.Q_pack_Ah / 10) * ones(1, N);

    [x_true, y_true] = simulate_true(params, I, dt);
    y_noisy = y_true + 0.02*randn(size(y_true));

    ekf_out = batch_ekf_estimator(params, I, y_noisy, dt);
    plot_comprehensive_analysis(time, x_true, y_true, y_noisy, ekf_out, I, 'Long Stress Discharge');
end

function [x_true, y_true] = simulate_true(params, I, dt)
    N = numel(I);
    x_true = zeros(2, N);
    y_true = zeros(1, N);
    x_true(:,1) = [1; 0];
    y_true(1) = ssm_output_fn(x_true(:,1), I(1), params);
    for k = 2:N
        x_true(:,k) = ssm_state_update(x_true(:,k-1), I(k-1), params, dt);
        y_true(k) = ssm_output_fn(x_true(:,k), I(k), params);
    end
end

function plot_comprehensive_analysis(time, x_true, y_true, y_noisy, ekf_out, I, title_str)
    fig = figure('Name', ['EKF Analysis: ', title_str], 'Position', [50 50 1800 1200]);
    
    soc_error = x_true(1,:) - ekf_out.x_hat(1,:);
    voltage_error = y_true - ekf_out.y_pred;
    rmse_soc = sqrt(mean(soc_error.^2));
    rmse_voltage = sqrt(mean(voltage_error.^2));
    max_abs_soc_error = max(abs(soc_error));
    
    subplot(3,3,1);
    h1 = plot(time/3600, x_true(1,:)*100, 'LineWidth', 3, 'Color', [0 0.4470 0.7410]);
    hold on;
    h2 = plot(time/3600, ekf_out.x_hat(1,:)*100, '--', 'LineWidth', 2.5, 'Color', [0.8500 0.3250 0.0980]);
    title('SOC Comparison', 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('Time [h]', 'FontSize', 11);
    ylabel('SOC [%]', 'FontSize', 11);
    legend([h1 h2], {'True SOC', 'EKF Estimate'}, 'Location', 'best', 'FontSize', 10);
    grid on; grid minor;
    ylim([-5 105]);
    
    subplot(3,3,2);
    h1 = plot(time/3600, y_true, 'LineWidth', 2.5, 'Color', [0 0.4470 0.7410]);
    hold on;
    h2 = plot(time/3600, y_noisy, '.', 'MarkerSize', 4, 'Color', [0.7 0.7 0.7]);
    h3 = plot(time/3600, ekf_out.y_pred, '--', 'LineWidth', 2, 'Color', [0.8500 0.3250 0.0980]);
    title('Voltage Comparison', 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('Time [h]', 'FontSize', 11);
    ylabel('Voltage [V]', 'FontSize', 11);
    legend([h1 h2 h3], {'True Voltage', 'Noisy Measurements', 'EKF Prediction'}, 'Location', 'best', 'FontSize', 10);
    grid on; grid minor;
    
    subplot(3,3,3);
    plot(time/3600, I, 'LineWidth', 2, 'Color', [0.4940 0.1840 0.5560]);
    title('Current Profile', 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('Time [h]', 'FontSize', 11);
    ylabel('Current [A]', 'FontSize', 11);
    grid on; grid minor;
    
    subplot(3,3,4);
    plot(time/3600, soc_error*100, 'LineWidth', 2, 'Color', [0.9290 0.6940 0.1250]);
    title(sprintf('SOC Error (RMSE: %.3f%%, Max: %.3f%%)', rmse_soc*100, max_abs_soc_error*100), 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('Time [h]', 'FontSize', 11);
    ylabel('SOC Error [%]', 'FontSize', 11);
    grid on; grid minor;
    yline(0, 'k--', 'Alpha', 0.5);
    
    subplot(3,3,5);
    plot(time/3600, voltage_error*1000, 'LineWidth', 2, 'Color', [0.4660 0.6740 0.1880]);
    title(sprintf('Voltage Error (RMSE: %.1f mV)', rmse_voltage*1000), 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('Time [h]', 'FontSize', 11);
    ylabel('Voltage Error [mV]', 'FontSize', 11);
    grid on; grid minor;
    yline(0, 'k--', 'Alpha', 0.5);
    
    subplot(3,3,6);
    plot(time/3600, ekf_out.innov*1000, 'LineWidth', 1.5, 'Color', [0.3010 0.7450 0.9330]);
    title('Innovation Sequence', 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('Time [h]', 'FontSize', 11);
    ylabel('Innovation [mV]', 'FontSize', 11);
    grid on; grid minor;
    yline(0, 'k--', 'Alpha', 0.5);
    
    subplot(3,3,7);
    if isfield(ekf_out, 'P_trace')
        P_soc = squeeze(ekf_out.P_trace(1,1,:));
        P_voltage = squeeze(ekf_out.P_trace(2,2,:));
        yyaxis left;
        plot(time/3600, sqrt(P_soc)*100, 'LineWidth', 2, 'Color', [0.6350 0.0780 0.1840]);
        ylabel('SOC Uncertainty [%]', 'FontSize', 11);
        yyaxis right;
        plot(time/3600, sqrt(P_voltage)*1000, 'LineWidth', 2, 'Color', [0 0.4470 0.7410]);
        ylabel('Voltage Uncertainty [mV]', 'FontSize', 11);
    else
        text(0.5, 0.5, 'Uncertainty data not available', 'HorizontalAlignment', 'center', 'Units', 'normalized');
    end
    title('State Uncertainties (±1σ)', 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('Time [h]', 'FontSize', 11);
    grid on; grid minor;
    
    subplot(3,3,8);
    scatter(x_true(1,:)*100, ekf_out.x_hat(1,:)*100, 20, time/3600, 'filled');
    hold on;
    plot([0 100], [0 100], 'k--', 'LineWidth', 2);
    cb = colorbar();
    ylabel(cb, 'Time [h]');
    title('SOC: Truth vs Estimate', 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('True SOC [%]', 'FontSize', 11);
    ylabel('Estimated SOC [%]', 'FontSize', 11);
    grid on; grid minor;
    axis equal;
    xlim([0 100]); ylim([0 100]);
    
    subplot(3,3,9);
    histogram(soc_error*100, 30, 'FaceColor', [0.8500 0.3250 0.0980], 'FaceAlpha', 0.7, 'EdgeColor', 'none');
    hold on;
    xline(mean(soc_error)*100, 'r--', 'LineWidth', 2, 'Label', sprintf('Mean: %.3f%%', mean(soc_error)*100));
    xline(0, 'k--', 'LineWidth', 1);
    title('SOC Error Distribution', 'FontSize', 12, 'FontWeight', 'bold');
    xlabel('SOC Error [%]', 'FontSize', 11);
    ylabel('Frequency', 'FontSize', 11);
    grid on; grid minor;
    
    sgtitle(sprintf('EKF Performance Analysis: %s', title_str), 'FontSize', 16, 'FontWeight', 'bold');
    
    print_performance_metrics(rmse_soc, rmse_voltage, max_abs_soc_error, mean(soc_error), std(soc_error));
end

function print_performance_metrics(rmse_soc, rmse_voltage, max_abs_soc_error, mean_soc_error, std_soc_error)
    fprintf('\n=== EKF Performance Metrics ===\n');
    fprintf('SOC RMSE:           %.4f%% (%.6f)\n', rmse_soc*100, rmse_soc);
    fprintf('SOC Max Error:      %.4f%% (%.6f)\n', max_abs_soc_error*100, max_abs_soc_error);
    fprintf('SOC Mean Error:     %.4f%% (%.6f)\n', mean_soc_error*100, mean_soc_error);
    fprintf('SOC Std Error:      %.4f%% (%.6f)\n', std_soc_error*100, std_soc_error);
    fprintf('Voltage RMSE:       %.2f mV (%.6f V)\n', rmse_voltage*1000, rmse_voltage);
    fprintf('===============================\n\n');
end
