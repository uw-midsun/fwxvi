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

function ocv = ocv_from_soc(params, soc) %#codegen
    coder.inline('always');

    % Clamp SOC to [0,1] and convert for lookup indexing
    soc_lookup = 1 - min(max(soc, 0), 1);  
    
    soc_tab = params.SOC_table;
    ocv_tab = params.OCV_table;
    N = params.SOC_OCV_table_size;
    
    % --- Interpolation / Clamping ---
    if soc_lookup <= soc_tab(1)
        ocv_cell = ocv_tab(1);
    elseif soc_lookup >= soc_tab(N)
        ocv_cell = ocv_tab(N);
    else
        % Replace find with a loop
        idx = -1;
        for k = 1:N-1
            if soc_lookup >= soc_tab(k) && soc_lookup <= soc_tab(k+1)
                idx = k;
                break;
            end
        end
        if idx == -1
            idx = N - 1;
        end

        s0 = soc_tab(idx); 
        s1 = soc_tab(idx+1);
        v0 = ocv_tab(idx); 
        v1 = ocv_tab(idx+1);

        if (s1 - s0) == 0
            ocv_cell = v0;
        else
            ocv_cell = v0 + (v1 - v0) * ((soc_lookup - s0) / (s1 - s0));
        end
    end

    % --- Scale to pack ---
    if isfield(params, 'N_series')
        % Ensure N_series is scalar double
        N_series = double(params.N_series);
        if ~isscalar(N_series)
            N_series = N_series(1); % fallback if needed
        end
        ocv = N_series * double(ocv_cell);
    else
        ocv = double(ocv_cell); % fallback to cell voltage if N_series missing
    end
end
