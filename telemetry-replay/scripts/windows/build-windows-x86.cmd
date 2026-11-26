@echo off
call "%~dp0build-windows.cmd" --x86 %*
exit /b %errorlevel%
