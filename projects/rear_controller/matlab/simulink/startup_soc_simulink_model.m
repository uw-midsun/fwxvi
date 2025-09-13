disp('Running startup_bus.m to create battery_bus');

battery_bus = define_ekf_params_bus();
assignin('base', 'battery_bus', battery_bus);

params_struct = battery_params();

% Remove function handles before assigning to Simulink if needed
params_struct = rmfield(params_struct, {'ocv_fun', 'ocv_grad'});

assignin('base', 'params_struct', params_struct);

addpath('..');
addpath('../utils');
addpath('../state_space_model');
