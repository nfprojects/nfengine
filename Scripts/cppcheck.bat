@echo off

:: make sure that cppcheck is reachable
echo | set /p=Checking for cppcheck... 
for %%X in (cppcheck.exe) do (set FOUND=%%~$PATH:X)
if defined FOUND (
    echo FOUND
) else (
    echo NOT FOUND
    echo     Make sure you download cppcheck before using this script.
    echo     See README.md for more info. Exiting.
    pause
    exit /b
)

pushd .
cd %~dp0\..
echo Current dir is %cd%

if "%1" == "quiet" (
    cppcheck ./Src/ -q -j 8 --enable=warning 2> cppcheck_result.txt
) else (
    cppcheck ./Src/ -j 8 --enable=warning 2> cppcheck_result.txt
)

popd
