## How to run

Run autogen from the repository root:

```bash
python3 -m autogen <template_name> -o <output_dir>
```

## Supported template names

- `system_can`
- `project_can`
- `simulation_app`
- `cantools`
- `new_project`

## Commands for each generator

### 1) Generate shared `system_can` headers

```bash
python3 -m autogen system_can -o can/inc
python3 -m autogen system_can -o mpxe/server/app/inc
```

Outputs:
- `system_can.h`

### 2) Generate per-project CAN interface code

Example for front controller object directory:

```bash
python3 -m autogen project_can -o build/arm/obj/front_controller/can
```

Outputs:
- `inc/can_codegen.h`
- `inc/<project_name>_filters.h`
- `inc/<project_name>_getters.h`
- `inc/<project_name>_rx_structs.h`
- `inc/<project_name>_setters.h`
- `inc/<project_name>_tx_structs.h`
- `src/<project_name>_rx_all.c`
- `src/<project_name>_tx_all.c`

### 3) Generate MPXE simulation app CAN files

```bash
python3 -m autogen simulation_app -o mpxe/server/app
```

Outputs:
- `inc/can_scheduler.h`
- `src/can_message_handler.cc`
- `src/can_scheduler.cc`

### 4) Generate CAN Python tooling and DBC

```bash
python3 -m autogen cantools -o can/tools
```

Outputs:
- `can_simulator.py`
- `can_simulator_cache.py`
- `system_can.py`
- `system_dbc.dbc`

### 5) Generate new targets (`projects`, `smoke`, `py`)

Use `new_project` template name. The output path decides which template is used:

```bash
# New firmware project
python3 -m autogen new_project -o projects/<project_name>

# New smoke target
python3 -m autogen new_project -o smoke/<smoke_name>

# New python package/module target
python3 -m autogen new_project -o py/<module_name>
```

Outputs by target type:

`projects/<project_name>`:
- `config.json`
- `README.md`
- `inc/<project_name>.h`
- `src/main.c`
- `test/test_<project_name>.c`

`smoke/<smoke_name>`:
- `config.json`
- `README.md`
- `src/main.c`

`py/<module_name>`:
- `config.json`
- `main.py`
- `__init__.py`
- `<module_name>.py`
