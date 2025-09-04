function params = battery_params()
    params.Q_pack_Ah = 4.5;         % nominal capacity in Ah
    params.R0        = 0.015;       % Internal ohmic resistance (Ohm)
    params.R1        = 0.05;        % RC resistor (Ohm)
    params.C1        = 2000;        % RC capacitance (Farads) -> R1*C1 = tau

    % Load 0.2C discharge curve from data/0_2_voltage_soc_curve.csv
    % Expect columns: Capacity_mAH, OCV
    ocv_table = readmatrix(fullfile('data','discharge_curve.csv'));

    cap_mAh = ocv_table(:,1);        % [mAh]
    V_ocv   = ocv_table(:,2);        % [V]
    soc     = cap_mAh / (params.Q_pack_Ah * 1000);

    params.SOC_table = soc;
    params.OCV_table = V_ocv;

    params.ocv_fun  = @(soc) interp1(params.SOC_table, params.OCV_table, ...
                                     min(max(soc,0),1), 'linear','extrap');
    params.ocv_grad = @(soc) gradient(params.OCV_table, params.SOC_table) ...
                                     (interp1(params.SOC_table, ...
                                              1:numel(params.SOC_table), ...
                                              min(max(soc,0),1), 'nearest','extrap'));

end
