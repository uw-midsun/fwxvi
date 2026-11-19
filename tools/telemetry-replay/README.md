# Midnight Sun Telemetry Replay

A desktop application for viewing CAN telemetry from car SD-card logs. This project is being developed for Midnight Sun issue #271.

## Install or update a prebuilt Windows version

1. Open [GitHub Releases](https://github.com/uw-midsun/midsun-telemetry-replay/releases). If a prebuilt ZIP is available under **Assets**, download the one for your Windows architecture. Choose **ARM64** for Windows on Apple Silicon via Parallels or **x64** for 64-bit Intel and AMD PCs.
2. Close the running app and extract the **whole ZIP** to a new folder on your Windows drive. Keep `telemetry.exe`, the Qt DLLs, and the plugin folders together.
3. Open `telemetry.exe` from the new folder. Keep your telemetry logs in a separate folder so both versions can open them.

If a prebuilt ZIP is unavailable for your architecture, build from source using the steps below.

## Update a source checkout

1. Close any running copy of Telemetry Replay before rebuilding, since Windows can lock the executable while it is running.
2. Save any local edits. From the repository root, update the checked-out branch with `git pull --ff-only`.
3. Rerun the build command for your platform under [Build and run](#build-and-run), then launch the rebuilt executable.

In Parallels, build from a checkout on the Windows drive, such as `C:\Dev\midsun-telemetry-replay`. MSBuild can report dependency-path warnings on the Mac shared drive. You can still open telemetry logs from the shared drive in the app.

## Requirements

- A C++17 compiler
- CMake 3.16 or newer
- Qt 6 with the Widgets module

## Build and run

### macOS

From the project directory, configure, build, and launch the app.

```sh
cmake -S . -B build
cmake --build build
./build/telemetry
```

If CMake cannot find your Homebrew Qt installation, configure with its install path.

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
```

### Windows

Install **Visual Studio 2022** or **Build Tools 2022** with the **Desktop development with C++** workload and a Windows SDK. Use CMake 3.21 or newer, either through the **C++ CMake tools for Windows** component or as a standalone installation on PATH.

Install the Qt kit for your target, then run the matching command from PowerShell or Command Prompt in the repository root.

| Target | Qt and compiler setup | Build command | Executable |
| --- | --- | --- | --- |
| x64 (64-bit Intel or AMD) | Qt 6 **MSVC 2022 64-bit** desktop kit | `.\scripts\windows\build-windows.cmd` | `build\windows\Release\telemetry.exe` |
| ARM64 (including Windows on Apple Silicon via Parallels) | MSVC v143 ARM64 build tools, Qt 6.8 or newer **MSVC ARM64** desktop kit, and the matching MSVC x64 Qt host kit alongside it | `.\scripts\windows\build-windows-arm64.cmd` | `build\windows-arm64\Release\telemetry.exe` |
| x86 (32-bit) | MSVC x86 tools and a custom 32-bit MSVC Qt 6 Widgets kit with `windeployqt.exe` | `.\scripts\windows\build-windows-x86.cmd "D:\Qt\msvc-x86"` | `build\windows-x86\Release\telemetry.exe` |

The scripts compile a Release build and bundle the Qt runtime. Double-click the executable to open the app. To share it, zip the entire corresponding `Release` folder.

The scripts look for Qt kits under `C:\Qt`. For a custom installation, pass the full kit folder as an argument, such as `.\scripts\windows\build-windows.cmd "D:\Qt\6.8.3\msvc2022_64"`. Replace the example path with your actual kit path.

## Using the application

1. Select **Open file** and choose a telemetry log.
2. View parsed messages in the table.
3. Move the timeline slider or select a row to see the raw payload and, for supported messages, its decoded signals.

The slider selects messages in file order. To try the app, open [the synthetic test drive](samples/midsun_synthetic_test_drive.log) or pass it as the first argument when launching the app.

```sh
./build/telemetry samples/midsun_synthetic_test_drive.log
```

```powershell
.\build\windows-arm64\Release\telemetry.exe .\samples\midsun_synthetic_test_drive.log
```

The Windows example uses the ARM64 build. Use the corresponding executable path from the table for x64 or x86.

## Log format

The parser currently expects one CAN message per line, for example:

```text
(1.251000) can0 035 #DAF97EAAF4C8
```

The fields are the timestamp in seconds, interface name, decimal CAN ID, and hexadecimal payload. Lines that do not match the supported format are skipped. The payload can contain up to 8 bytes.

## Project structure

| Path | Contents |
| --- | --- |
| `assets/` | Original team logo and the PNG embedded in the app |
| `can/` | CAN board definitions, DBC, and supporting code |
| `samples/` | Example telemetry logs |
| `src/gui/` | Main window, frame table, and timeline |
| `src/telemetry/` | Log parsing and signal decoding |
| `scripts/windows/` | Windows build and packaging scripts |
| `build/` | Local build outputs, ignored by Git |
