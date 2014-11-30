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
    exit /b
)


:: make sure that we are working in our repository:
::   1. set current directory
echo Current dir is %cd%
echo Current launch dir is %~dp0

pushd .
cd %~dp0\..

echo Current dir is %cd%

 
::   2. search for Engine.sln
echo | set /p=Locating Engine.sln... 
for /r . %%G in (*) do (
    if "%%~nxG"=="Engine.sln" (
        set slnPath=%%~dpnxG
        goto :break
    )
)
:break

if defined slnPath (
    echo FOUND
    echo     Engine.sln found in %slnPath%
) else (
    echo NOT FOUND
    echo     Cannot find root nfEngine directory. Exiting.
    popd
    exit /b
)


:: launch astyle for all projects
echo.
echo.
echo Launching astyle
astyle --options=Scripts\style.cfg -r -v --suffix=none nfEngine\*.cpp nfEngine\*.hpp
echo.
:: Prerequisites.hpp should have namespaces indented to improve readability
:: run astyle once more specifically on this file
astyle --options=Scripts\style.cfg -v --suffix=none --indent-namespaces nfEngine\nfCore\Prerequisites.hpp

popd
