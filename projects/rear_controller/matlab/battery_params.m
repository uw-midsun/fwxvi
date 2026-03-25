% =========================================================================
% @file    battery_params.m
% @brief   Battery cell + pack parameter definition and OCV table loading
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Defines nominal *cell-level* parameters, pack topology (series/parallel),
% and computes *pack-level* equivalents. Also loads OCV vs SOC lookup data
%
% -------------------------------------------------------------------------
% OUTPUT:
%   params : struct
%       Struct containing:
%         -- Topology --
%         - N_series, N_parallel
%
%         -- Cell-level params (from datasheet/test) --
%         - Q_cell_Ah   : nominal cell capacity [Ah]
%         - R0_cell     : ohmic resistance [Ohm]
%         - R1_cell     : RC resistor [Ohm]
%         - C1_cell     : RC capacitance [F]
%
%         -- Derived pack-level params --
%         - Q_pack_Ah   : pack capacity [Ah]
%         - R0_pack     : equivalent ohmic resistance [Ohm]
%         - R1_pack     : equivalent RC resistance [Ohm]
%         - C1_pack     : equivalent RC capacitance [F]
%
%         -- EKF params --
%         - Q_proc      : process noise covariance
%         - R_meas      : measurement noise variance
%         - rest_thresh : current threshold for rest detection [A]
%         - rest_window : window length for rest detection [steps]
%         - alpha       : blending factor for soft reset
%
%         -- OCV table --
%         - SOC_table   : SOC samples (0-1)
%         - OCV_table   : OCV samples [V] (cell-level)
%         - ocv_fun     : function handle, OCV_cell(SOC)
%         - ocv_grad    : function handle, dOCV/dSOC(SOC)
%
% NOTES:
%   - The OCV curve is defined at *cell level*. For pack, multiply OCV by N_series.
%
% =========================================================================

function params = battery_params() %#codegen
    params = struct();

    % --- Pack topology ---
    params.N_series   = 36;
    params.N_parallel = 9;

    % --- Cell-level parameters ---
    params.Q_cell_Ah = 4.5;     % [Ah]
    params.R0_cell   = 0.015;   % [Ohm]
    params.R1_cell   = 0.05;    % [Ohm]
    params.C1_cell   = 2000;    % [F]

    % --- Derived pack-level parameters ---
    params.Q_pack_Ah = params.N_parallel * params.Q_cell_Ah;
    params.R0_pack   = params.N_series * (params.R0_cell / params.N_parallel);
    params.R1_pack   = params.N_series * (params.R1_cell / params.N_parallel);
    params.C1_pack   = (params.N_parallel / params.N_series) * params.C1_cell;

    % --- Noise covariances ---
    params.Q_proc = single([1e-5 0; 0 1e-5]); % process noise covariance
    params.R_meas = single(1e-2);             % measurement noise variance

    % --- EKF reset parameters ---
    params.rest_thresh = single(0.05);   % current [A] below which battery is "at rest"
    params.rest_window = int32(100);     % number of timesteps of rest before reset
    params.alpha       = single(0.05);   % Alpha factor for soft reset

    % --- Load OCV vs SOC (cell-level) ---
    scriptPath = fileparts(mfilename('fullpath'));
    csvPath = fullfile(scriptPath, 'data', '0_2_voltage_soc_curve.csv');
    ocv_table = readmatrix(csvPath);

    cap_mAh = ocv_table(:,1);                       % [mAh]
    V_ocv   = ocv_table(:,2);                       % [V]
    soc     = cap_mAh / (params.Q_cell_Ah * 1000); % normalize to cell capacity

    % Downsample to 100 points for embedded target
    params.SOC_OCV_table_size = 100;
    idx = round(linspace(1, length(soc), params.SOC_OCV_table_size));  % even spacing indices

    params.SOC_table = soc(idx);
    params.OCV_table = V_ocv(idx);

    % Interpolants
    params.ocv_fun  = @(soc_query) interp1(params.SOC_table, params.OCV_table, soc_query, 'linear', 'extrap');
    params.ocv_grad = @(soc_query) gradient(params.ocv_fun(soc_query), soc_query);

    % Load tables into C header files
    exportDir = fullfile(scriptPath, '..', 'inc');
    if ~exist(exportDir, 'dir')
        mkdir(exportDir);
    end

    headerPath = fullfile(exportDir, 'state_of_charge_lut.h');
    fid = fopen(headerPath, 'w');

    % --- Doxygen / header block ---
    fprintf(fid, '#pragma once\n\n');
    fprintf(fid, '/************************************************************************************************\n');
    fprintf(fid, ' * @file    state_of_charge_lut.h\n');
    fprintf(fid, ' *\n');
    fprintf(fid, ' * @brief   Auto-generated SOC–OCV lookup table for the State of Charge estimator\n\n');
    fprintf(fid, ' * @date    %s\n', datestr(now, 'yyyy-mm-dd'));
    fprintf(fid, ' * @author  ');
    fprintf(fid, 'Midnight Sun Team #24 - MSXVI\n');
    fprintf(fid, ' ************************************************************************************************/\n\n');

    fprintf(fid, '/**\n');
    fprintf(fid, ' * @defgroup Rear_Controller\n');
    fprintf(fid, ' * @brief    Rear Controller Board Firmware\n');
    fprintf(fid, ' * @{\n');
    fprintf(fid, ' */\n\n');

    % --- Table size define ---
    fprintf(fid, '#define SOC_OCV_TABLE_SIZE (%d)\n\n', params.SOC_OCV_table_size);

    % --- SOC table define ---
    fprintf(fid, '/** @brief SOC table initializer list */\n');
    fprintf(fid, '#define SOC_TABLE_VALUES { \\\n');
    for i = 1:params.SOC_OCV_table_size
        fprintf(fid, '    %.6ff', params.SOC_table(i));
        if i < params.SOC_OCV_table_size
            fprintf(fid, ',');
        end
        if mod(i, 8) == 0 || i == params.SOC_OCV_table_size
            fprintf(fid, ' \\\n');
        else
            fprintf(fid, ' ');
        end
    end
    fprintf(fid, '}\n\n');

    % --- OCV table define ---
    fprintf(fid, '/** @brief OCV table initializer list */\n');
    fprintf(fid, '#define OCV_TABLE_VALUES { \\\n');
    for i = 1:params.SOC_OCV_table_size
        fprintf(fid, '    %.6ff', params.OCV_table(i));
        if i < params.SOC_OCV_table_size
            fprintf(fid, ',');
        end
        if mod(i, 8) == 0 || i == params.SOC_OCV_table_size
            fprintf(fid, ' \\\n');
        else
            fprintf(fid, ' ');
        end
    end
    fprintf(fid, '}\n\n');

    fprintf(fid, '/** @} */\n');

    fclose(fid);
    fprintf('C header file generated: %s\n', headerPath);
end
