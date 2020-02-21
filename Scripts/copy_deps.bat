@echo off

:: WARNING
:: This script was ported in a "just in case" matter.
:: It was not heavily tested, as we still don't know if we'll ever use it.

:: script assumes following arguments:
::   %1   - PlatformDirName
::   %2   - DestConfigurationDirName
::   %4   - SrcConfigurationDirName
::   %4.. - <list of files to copy>

:: verify that some arguments were given
if "%4"=="" (
    echo ERROR: There were not enough arguments given. Was that intentional?
    echo        Leaving script.
    exit /b
)

:: go to base nfEngine dir
pushd .
cd %~dp0\..

echo Current dir is %cd%

:: verify if deps are downloaded and compiled
if not exist "nfEngineDeps/Bin" (
    echo ERROR: nfEngineDeps is not downloaded/compiled.
    echo        Please, download and compile nfEngineDeps first. See README.md for more info.
    popd
    exit /b
)

set BasePath=%cd%\nfEngineDeps\Bin\%1\%3\
set DestPath=%cd%\Bin\%1\%2\
echo Deps files will be copied from %BasePath% to %DestPath%

:mainloop
if "%4"=="" (
    goto continue
)

if exist "%BasePath%%4" (
    echo Copying %4 from %BasePath%
    copy "%BasePath%%4" "%DestPath%\%~nx4"
) else (
    echo ERROR: %4 not found - skipping...
    set Failed=true
)
shift
goto mainloop
:continue

popd

if defined Failed: (
    echo Errors were found while copying dependencies.
    exit 1
) else (
    echo Copy operation finished successfully.
)
