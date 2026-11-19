@echo off
call "%~dp0build-windows.cmd" --arm64 %*
exit /b %errorlevel%
