% =========================================================================
% @file    ocv_to_soc.m
% @brief   Inverse lookup: SOC from open-circuit voltage
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Computes SOC from a measured OCV using inverse interpolation.
% Assumes monotonic OCV_table. Converts back from (1 - SOC) indexing
%
% -------------------------------------------------------------------------
% INPUTS:
%   params  : struct
%       Battery parameters with fields:
%           - SOC_table : [Nx1] vector
%           - OCV_table : [Nx1] vector
%   v       : double [scalar]
%       Open-circuit voltage [V]
%
% OUTPUT:
%   soc     : double [scalar]
%       Estimated SOC [0-1]
%
% =========================================================================

function soc = ocv_to_soc(params, v)
    v_cell = double(v) / double(params.N_series);

    soc_tab = params.SOC_table;
    ocv_tab = params.OCV_table;
    N = numel(ocv_tab);

    if v_cell <= ocv_tab(1)
        soc = 1 - soc_tab(1); return;  % Convert back to actual SOC
    elseif v_cell >= ocv_tab(N)
        soc = 1 - soc_tab(N); return;  % Convert back to actual SOC
    end

    idx = 1;
    for i = 1:(N-1)
        if v_cell >= ocv_tab(i) && v_cell <= ocv_tab(i+1)
            idx = i; break;
        end
    end

    v0 = ocv_tab(idx); v1 = ocv_tab(idx+1);
    s0 = soc_tab(idx); s1 = soc_tab(idx+1);

    if (v1 - v0) == 0
        soc = 1 - s0;  % Convert back to actual SOC
    else
        soc_interp = s0 + (s1 - s0) * ((v_cell - v0) / (v1 - v0));
        soc = 1 - soc_interp;  % Convert back to actual SOC
    end
end
