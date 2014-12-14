@echo off

:: make sure that astyle is reachable
echo | set /p=Checking for astyle... 
for %%X in (astyle.exe) do (set FOUND=%%~$PATH:X)
if defined FOUND (
    echo FOUND
) else (
    echo NOT FOUND
    echo     Make sure you download astyle before using this script.
    echo     See README.md for more info. Exiting.
    pause
    exit /b
)


:: make sure that we are working in our repository
pushd .
cd %~dp0\..
echo Current dir is %cd%

if "%1" == "dry" (
    echo Triggering astyle in dry run mode
    echo.
    :: launch astyle for all projects
    astyle --options=Scripts\style.cfg -r -v --dry-run --formatted --suffix=none --exclude=nfEngine\nfCore\Prerequisites.hpp nfEngine\*.cpp nfEngine\*.hpp
    echo.
    :: Prerequisites.hpp should have namespaces indented to improve readability
    :: run astyle once more specifically on this file
    astyle --options=Scripts\style.cfg -v --dry-run --formatted --suffix=none --indent-namespaces nfEngine\nfCore\Prerequisites.hpp
) else (
    echo Triggering astyle
    echo.
    :: launch astyle for all projects
    astyle --options=Scripts\style.cfg -r -v --suffix=none --exclude=nfEngine\nfCore\Prerequisites.hpp nfEngine\*.cpp nfEngine\*.hpp
    echo.
    :: Prerequisites.hpp should have namespaces indented to improve readability
    :: run astyle once more specifically on this file
    astyle --options=Scripts\style.cfg -v --suffix=none --indent-namespaces nfEngine\nfCore\Prerequisites.hpp
)

popd
