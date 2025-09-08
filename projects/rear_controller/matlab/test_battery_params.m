% =========================================================================
% @file    test_battery_params.m
% @brief   Diagnostic test for battery_params definitions and OCV functions
% @date    2025-09-07
% @autho   Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Verifies that OCV vs SOC functions behave as expected. Prints sample
% values, checks monotonicity, and generates diagnostic plots:
%   1. Raw OCV vs SOC from CSV
%   2. Interpolated OCV function output
%   3. Gradient dOCV/dSOC
%
% -------------------------------------------------------------------------
% INPUTS:
%   (none)
%
% OUTPUTS:
%   (none, diagnostic only)
%
% SIDE EFFECTS:
%   - Prints table of OCV values at coarse SOC points
%   - Prints PASS/ERROR diagnostic message
%   - Opens MATLAB figure with three subplots
%
% =========================================================================

function test_battery_params()
    addpath('utils');

    params = battery_params();
    Q_mAh = params.Q_pack_Ah * 1000;

    % Define SOC test values
    soc_coarse = linspace(0, 1, 11);    % Coarse SOC points for display
    soc_fine   = linspace(0, 1, 1000);  % Fine SOC points for plotting

    % Compute OCV values using the utility functions (ocv_from_soc)
    ocv_fine = arrayfun(@(soc) ocv_from_soc(params, soc), soc_fine);  % OCV from SOC

    % SOC from OCV to check inverse functionality
    soc_fine_from_ocv = arrayfun(@(ocv) ocv_to_soc(params, ocv), ocv_fine);  % SOC from OCV
    
    % Calculate the gradient (dOCV/dSOC) using finite differences
    docv_dsoc = diff(ocv_fine) ./ diff(soc_fine);
    docv_dsoc = [docv_dsoc(1), docv_dsoc];

    % ---------- Print table of OCV values and gradient ----------    
    fprintf('SOC   |  OCV [V]  | dOCV/dSOC\n');
    fprintf('------+-----------+-----------\n');

    for i = 1:length(soc_coarse)
        ocv = ocv_from_soc(params, soc_coarse(i));  % Convert SOC to OCV

        % Approximate gradient (assuming small steps in SOC)
        grad = (ocv_from_soc(params, soc_coarse(i) + 0.01) - ocv) / 0.01;  % 1% change in SOC
        fprintf('%4.1f |   %6.3f  |   %+6.3f\n', soc_coarse(i), ocv, grad);
    end

    % ---------- Basic check for monotonicity of OCV ----------    
    if ocv_from_soc(params, 1.0) < ocv_from_soc(params, 0.0)
        fprintf('\nERROR: Inverted OCV mapping detected!\n');
        fprintf('   SOC=1.0 → %.3f V, SOC=0.0 → %.3f V\n', ...
            ocv_from_soc(params, 1.0), ocv_from_soc(params, 0.0));
    else
        fprintf('\nPASS: OCV function increases with SOC as expected.\n');
    end

    % ---------- Plotting ----------    
    figure('Name','Battery OCV Diagnostic','Position',[100 100 1400 500]);

    % Plot 1: Raw OCV vs SOC from CSV
    subplot(1,3,1);
    plot(params.SOC_table, params.OCV_table, 'bo-', 'LineWidth', 1.5);
    xlabel('SOC (from CSV)');
    ylabel('Cell OCV [V]');
    title('Raw Cell OCV vs SOC');
    grid on;

    % Plot 2: Evaluated OCV function using ocv_from_soc
    subplot(1,3,2);
    plot(soc_fine, ocv_fine, 'r-', 'LineWidth', 2);
    xlabel('SOC input');
    ylabel('Pack OCV output [V]');
    title('Pack OCV Function Output');
    grid on;

    % Plot 3: Derivative dOCV/dSOC (gradient)
    subplot(1,3,3);
    plot(soc_fine, docv_dsoc, 'k-', 'LineWidth', 2);
    xlabel('SOC');
    ylabel('Pack dOCV/dSOC [V/unit SOC]');
    title('Pack OCV Slope (Gradient)');
    grid on;
end
