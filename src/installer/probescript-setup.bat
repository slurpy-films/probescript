@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

:main
cls
echo.
echo ========================================
echo            PROBESCRIPT SETUP
echo ========================================
echo.
echo Select an option:
echo.
echo [1] Install probescript
echo [2] Uninstall probescript
echo [3] Check installation status
echo [4] Exit
echo.
set /p choice="Enter your choice (1-4): "

if "%choice%"=="1" goto install
if "%choice%"=="2" goto uninstall
if "%choice%"=="3" goto check_status
if "%choice%"=="4" goto exit
echo Invalid choice. Try again.
pause
goto main

:install
cls
echo.
echo ========================================
echo          INSTALL PROBESCRIPT
echo ========================================
echo.

if exist "C:\Program Files\Probescript\probescript.exe" (
    echo Probescript is already installed in C:\Program Files\Probescript\
    echo.
    set /p reinstall="Do you want to reinstall? (y/n): "
    if /i not "!reinstall!"=="y" goto main
)

set "local_binary=%cd%\probescript.exe"
if exist "%local_binary%" (
    echo Found probescript.exe in current directory.
    set "binary_path=%local_binary%"
    goto continue_install
)

echo Enter path to probescript.exe:
set /p binary_path="Path (or press Enter to cancel): "

if "%binary_path%"=="" (
    echo No path provided. Cancelling installation.
    pause
    goto main
)

if not exist "%binary_path%" (
    echo Error: File does not exist.
    pause
    goto main
)

:continue_install
echo.
echo Default installation directory: C:\Program Files\Probescript
set /p install_dir="Enter installation directory (or press Enter for default): "

if "%install_dir%"=="" set "install_dir=C:\Program Files\Probescript"

echo.
echo Creating installation directory: %install_dir%
mkdir "%install_dir%" 2>nul
if errorlevel 1 (
    echo Error: Could not create installation directory.
    echo This may be due to insufficient administrator privileges.
    echo Try running this script as administrator.
    pause
    goto main
)

echo Copying probescript.exe to %install_dir%\
copy "%binary_path%" "%install_dir%\probescript.exe" >nul
if errorlevel 1 (
    echo Error: Could not copy file.
    pause
    goto main
)

echo Probescript copied to: %install_dir%\probescript.exe

echo.
set /p add_path="Add probescript to PATH? (y/n): "
if /i "%add_path%"=="y" (
    call :add_to_path "%install_dir%"
)

echo.
echo Installation completed!
call :show_ascii_art
echo.
echo Welcome to probescript!
echo To see a list of commands, run: probescript --help
echo Repository: https://github.com/slurpy-films/probescript
echo.
pause
goto main

:uninstall
cls
echo.
echo ========================================
echo         UNINSTALL PROBESCRIPT
echo ========================================
echo.

if not exist "C:\Program Files\Probescript\probescript.exe" (
    echo Probescript is not installed in standard location.
    echo Checking other possible locations...
    
    where probescript.exe >nul 2>&1
    if errorlevel 1 (
        echo Probescript was not found on the system.
        pause
        goto main
    ) else (
        echo Probescript found in PATH, but not in standard location.
        echo You must manually remove it from your PATH and delete the file.
        pause
        goto main
    )
)

echo Probescript found at: C:\Program Files\Probescript\
echo.
set /p confirm="Are you sure you want to uninstall probescript? (y/n): "
if /i not "%confirm%"=="y" goto main

call :remove_from_path "C:\Program Files\Probescript"

echo Deleting files...
del "C:\Program Files\Probescript\probescript.exe" 2>nul
rmdir "C:\Program Files\Probescript" 2>nul

if exist "C:\Program Files\Probescript" (
    echo Warning: Could not completely delete installation directory.
    echo You may need to delete it manually.
) else (
    echo Probescript has been uninstalled.
)

echo.
echo Uninstallation completed!
pause
goto main

:check_status
cls
echo.
echo ========================================
echo      PROBESCRIPT INSTALLATION STATUS
echo ========================================
echo.

if exist "C:\Program Files\Probescript\probescript.exe" (
    echo [OK] Probescript found at: C:\Program Files\Probescript\probescript.exe
    
    for %%F in ("C:\Program Files\Probescript\probescript.exe") do (
        echo     File size: %%~zF bytes
        echo     Last modified: %%~tF
    )
) else (
    echo [NOT FOUND] Probescript not found in standard location
)

echo.

echo Checking PATH...
where probescript.exe >nul 2>&1
if errorlevel 1 (
    echo [NOT IN PATH] Probescript not found in PATH
) else (
    echo [OK] Probescript found in PATH:
    where probescript.exe
)

echo.

echo Testing probescript...
probescript --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Could not run probescript
) else (
    echo [OK] Probescript runs as expected
    probescript --version 2>nul
)

echo.
pause
goto main

:add_to_path
set "new_path=%~1"
echo Adding %new_path% to PATH...

for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "current_path=%%B"

echo %current_path% | find /i "%new_path%" >nul
if not errorlevel 1 (
    echo PATH already contains probescript location.
    goto :eof
)

if "%current_path%"=="" (
    set "updated_path=%new_path%"
) else (
    set "updated_path=%current_path%;%new_path%"
)

reg add "HKCU\Environment" /v PATH /t REG_EXPAND_SZ /d "!updated_path!" /f >nul
if errorlevel 1 (
    echo Error: Could not update PATH.
) else (
    echo PATH updated. You may need to start a new command prompt.
)
goto :eof

:remove_from_path
set "remove_path=%~1"
echo Removing %remove_path% from PATH...

for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "current_path=%%B"

if "%current_path%"=="" (
    echo PATH is empty or not found.
    goto :eof
)

set "updated_path=!current_path!"
set "updated_path=!updated_path:%remove_path%;=!"
set "updated_path=!updated_path:;%remove_path%=!"
set "updated_path=!updated_path:%remove_path%=!"

reg add "HKCU\Environment" /v PATH /t REG_EXPAND_SZ /d "!updated_path!" /f >nul
if errorlevel 1 (
    echo Error: Could not update PATH.
) else (
    echo Removed from PATH.
)
goto :eof

:show_ascii_art
echo.
echo                 ________  ________  ________  ________  _______   ________  ________  ________  ___  ________  _________   
echo                ^|\   __  \^|\   __  \ \   __  \^|\   __  \^|\  ___ \ ^|\   ____\^|\   ____\^|\   __  \^|\  \^|\   __  \^|\___   ___\
echo                \ \  \^|\  \ \  \^|\  \ \  \^|\  \ \  \^|\ /\ \   __/ \ \  \___^|\ \  \___^|\ \  \^|\  \ \  \ \  \^|\  \^|___ \  \_^| 
echo                 \ \   ____\ \   _  _\ \  \\\  \ \   __  \ \  \   _\ \_____  \ \  \    \ \   _  _\ \  \ \   ____\   \ \  \
echo                  \ \  \___^|\ \  \\  \\ \  \\\  \ \  \^|\  \ \  \_^|\ \^|____^|\  \ \  \____\ \  \\  \\ \  \ \  \___^|    \ \  \
echo                   \ \__\    \ \__\\ _\\ \_______\ \_______\ \_______\____\_\  \ \_______\ \__\\ _\\ \__\ \__\        \ \__\
echo                    \^|__^|     \^|__^|\^|__^|\^|_______^|\^|_______^|\^|_______^|\_________\^|_______^|\^|__^|\^|__^|\^|__^|\^|__^|         \^|__^|
echo                                                                     \^|_______^|                                                                                                    
goto :eof

:exit
echo.
echo.
pause
exit /b 0