@echo off

:: make sure that doxygen is reachable
echo | set /p=Checking for doxygen... 
for %%X in (doxygen.exe) do (set FOUND=%%~$PATH:X)
if defined FOUND (
    echo FOUND
) else (
    echo NOT FOUND
    echo     Make sure you download doxygen before using this script.
    echo     See README.md for more info. Exiting.
    pause
    exit /b
)

pushd .
cd %~dp0\..
echo Current directory is %cd%
doxygen Scripts\doxygen.cfg
popd
