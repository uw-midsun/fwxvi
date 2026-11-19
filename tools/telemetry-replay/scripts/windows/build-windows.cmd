@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1

rem Use an explicit kit path for custom installations. Otherwise search C:\Qt.
set "platform=x64"
set "kitName=msvc2022_64"
set "buildDir=build\windows"
set "qtKit=%~1"
if /i "%~1"=="--arm64" (
    set "platform=ARM64"
    set "kitName=msvc2022_arm64"
    set "buildDir=build\windows-arm64"
    set "qtKit=%~2"
)
if /i "%~1"=="--x86" (
    set "platform=Win32"
    set "kitName=msvc2022_32"
    set "buildDir=build\windows-x86"
    set "qtKit=%~2"
)
if defined qtKit goto checkKit
for /f "delims=" %%V in ('dir /b /ad /o:-d "C:\Qt\6.*" 2^>nul') do (
    if exist "C:\Qt\%%V\%kitName%\lib\cmake\Qt6\Qt6Config.cmake" (
        set "qtKit=C:\Qt\%%V\%kitName%"
        goto checkKit
    )
)

:checkKit
if not defined qtKit goto missingKit
if not exist "%qtKit%\lib\cmake\Qt6\Qt6Config.cmake" goto missingKit
set "hostKit=%qtKit%"
if /i "%platform%"=="ARM64" if not exist "%qtKit%\bin\windeployqt.exe" set "hostKit=%qtKit%\..\msvc2022_64"
if not exist "%hostKit%\bin\windeployqt.exe" (
    echo Install the matching Qt host tools. Expected "%hostKit%\bin\windeployqt.exe".
    goto failed
)
set "deployArgs="
if /i not "%hostKit%"=="%qtKit%" set deployArgs=--qtpaths "%qtKit%\bin\qtpaths.bat"

where cmake >nul 2>nul && goto haveCMake
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%vswhere%" (
    for /f "usebackq delims=" %%I in (`"%vswhere%" -latest -products * -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`) do set "PATH=%%~dpI;%PATH%"
)
where cmake >nul 2>nul
if errorlevel 1 (
    echo Install CMake 3.21 or newer, add it to PATH, and reopen the terminal.
    goto failed
)

:haveCMake
set "PATH=%hostKit%\bin;%PATH%"
cmake -S . -B "%buildDir%" -G "Visual Studio 17 2022" -A %platform% "-DQT_HOST_PATH=%hostKit%" "-DQt6_DIR=%qtKit%\lib\cmake\Qt6" || goto failed
cmake --build "%buildDir%" --config Release --parallel || goto failed
"%hostKit%\bin\windeployqt.exe" %deployArgs% --release "%buildDir%\Release\telemetry.exe" || goto failed

echo Ready to run: "%CD%\%buildDir%\Release\telemetry.exe"
popd
exit /b 0

:missingKit
if defined qtKit (
    echo The selected folder "%qtKit%" is missing Qt6Config.cmake or windeployqt.exe.
) else (
    echo No %platform% Qt 6 desktop kit was found under C:\Qt\6.*\%kitName%.
)
echo For a custom installation, pass the kit directory containing bin and lib to the build script.
if /i "%platform%"=="x64" echo For x64, install the MSVC 2022 64-bit desktop kit with the Qt Online Installer or Qt Maintenance Tool.
if /i "%platform%"=="ARM64" echo For ARM64, install the MSVC 2022 ARM64 desktop kit with the Qt Online Installer or Qt Maintenance Tool.
if /i "%platform%"=="x64" if /i "%PROCESSOR_ARCHITECTURE%"=="ARM64" echo To target this ARM64 PC, run scripts\windows\build-windows-arm64.cmd from the repository root with the ARM64 kit.
:failed
popd
exit /b 1
