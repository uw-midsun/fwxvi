function battery_bus = define_ekf_params_bus()
    % Define bus elements
    elems = repmat(Simulink.BusElement, 1, 18); % 18 elements now (added SOC_OCV_table_size)

    names = {'N_series', 'N_parallel', 'Q_cell_Ah', 'R0_cell', 'R1_cell', 'C1_cell', ...
             'Q_pack_Ah', 'R0_pack', 'R1_pack', 'C1_pack', 'Q_proc', 'R_meas', ...
             'rest_thresh', 'rest_window', 'alpha', 'SOC_OCV_table_size', 'SOC_table', 'OCV_table'};

    dataTypes = {'double', 'double', 'double', 'double', 'double', 'double', ...
                 'double', 'double', 'double', 'double', 'double', 'double', ...
                 'double', 'int32', 'double', 'double', 'double', 'double'};  % SOC_OCV_table_size as double

    % Scalars up to element 10:
    for i = 1:10
        elems(i).Name = names{i};
        elems(i).DataType = dataTypes{i};
        elems(i).Dimensions = 1;
        elems(i).DimensionsMode = 'Fixed';
    end

    % Q_proc (11th element) 2x2 matrix:
    elems(11).Name = 'Q_proc';
    elems(11).DataType = 'double';
    elems(11).Dimensions = [2 2];
    elems(11).DimensionsMode = 'Fixed';

    % Scalars from 12 to 15
    for i = 12:15
        elems(i).Name = names{i};
        elems(i).DataType = dataTypes{i};
        elems(i).Dimensions = 1;
        elems(i).DimensionsMode = 'Fixed';
    end

    % SOC_OCV_table_size scalar
    elems(16).Name = 'SOC_OCV_table_size';
    elems(16).DataType = 'double';
    elems(16).Dimensions = 1;
    elems(16).DimensionsMode = 'Fixed';

    % SOC_table and OCV_table fixed-size vectors (100x1)
    elems(17).Name = 'SOC_table';
    elems(17).DataType = 'double';
    elems(17).Dimensions = [100 1];
    elems(17).DimensionsMode = 'Fixed';

    elems(18).Name = 'OCV_table';
    elems(18).DataType = 'double';
    elems(18).Dimensions = [100 1];
    elems(18).DimensionsMode = 'Fixed';

    % Create bus object and assign elements
    battery_bus = Simulink.Bus;
    battery_bus.Elements = elems;

    % Save bus object in base workspace
    assignin('base', 'battery_bus', battery_bus);
end
