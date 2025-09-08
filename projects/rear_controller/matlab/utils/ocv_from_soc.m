% =========================================================================
% @file    ocv_from_soc.m
% @brief   Lookup terminal open-circuit voltage (OCV) from SOC
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Uses linear interpolation on lookup tables to estimate OCV given SOC.
% Note: SOC_table is indexed by (1 - SOC).
%
% -------------------------------------------------------------------------
% INPUTS:
%   params  : struct
%       Battery parameters with fields:
%           - SOC_table : [Nx1] vector
%           - OCV_table : [Nx1] vector
%   soc     : double [scalar]
%       Actual state of charge [0-1]
%
% OUTPUT:
%   ocv     : double [scalar]
%       Estimated open-circuit voltage [V]
%
% =========================================================================

function ocv = ocv_from_soc(params, soc)
    % Clamp SOC to [0,1] and convert for lookup indexing
    soc_lookup = 1 - min(max(soc, 0), 1);  
    
    soc_tab = params.SOC_table;
    ocv_tab = params.OCV_table;
    N = numel(soc_tab);

    % --- Interpolation / Clamping ---
    if soc_lookup <= soc_tab(1)
        ocv_cell = ocv_tab(1);
    elseif soc_lookup >= soc_tab(N)
        ocv_cell = ocv_tab(N);
    else
        % Find interpolation index
        idx = find(soc_lookup >= soc_tab(1:end-1) & soc_lookup <= soc_tab(2:end), 1, 'first');
        if isempty(idx)
            idx = N-1;
        end

        s0 = soc_tab(idx); s1 = soc_tab(idx+1);
        v0 = ocv_tab(idx); v1 = ocv_tab(idx+1);

        if (s1 - s0) == 0
            ocv_cell = v0;
        else
            ocv_cell = v0 + (v1 - v0) * ((soc_lookup - s0) / (s1 - s0));
        end
    end

    % --- Scale to pack ---
    if isfield(params, 'N_series')
        ocv = double(params.N_series) * double(ocv_cell);
    else
        ocv = double(ocv_cell); % fallback to cell voltage if N_series missing
    end
end
