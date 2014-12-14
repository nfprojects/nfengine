@echo off

pushd .
cd %~dp0\..
echo Current directory is %cd%


:: nfCommon tests
echo.
echo ===== Running nfCommon tests =====
echo.
echo | set /p=Checking if tests are compiled for Win32/Debug... 
if exist Bin\Win32\Debug\nfCommonTest.exe (
    echo FOUND
    echo   Running nfCommon tests in Win32/Debug
    Bin\Win32\Debug\nfCommonTest.exe
) else (
    echo NOT FOUND
)

echo.
echo | set /p=Checking if tests are compiled for Win32/Release... 
if exist Bin\Win32\Release\nfCommonTest.exe (
    echo FOUND
    echo   Running nfCommon tests in Win32/Release
    Bin\Win32\Release\nfCommonTest.exe
) else (
    echo NOT FOUND
)

echo.
echo | set /p=Checking if tests are compiled for x64/Debug... 
if exist Bin\x64\Debug\nfCommonTest.exe (
    echo FOUND
    echo   Running nfCommon tests in x64/Debug
    Bin\x64\Debug\nfCommonTest.exe
) else (
    echo NOT FOUND
)

echo.
echo | set /p=Checking if tests are compiled for x64/Release... 
if exist Bin\x64\Release\nfCommonTest.exe (
    echo FOUND
    echo   Running nfCommon tests in x64/Release
    Bin\x64\Release\nfCommonTest.exe
) else (
    echo NOT FOUND
)


:: nfCore tests
echo.
echo ====== Running nfCore tests ======
echo.
echo | set /p=Checking if tests are compiled for Win32/Debug... 
if exist Bin\Win32\Debug\nfCoreTest.exe (
    echo FOUND
    echo   Running nfCore tests in Win32/Debug
    Bin\Win32\Debug\nfCoreTest.exe
) else (
    echo NOT FOUND
)

echo.
echo | set /p=Checking if tests are compiled for Win32/Release... 
if exist Bin\Win32\Release\nfCoreTest.exe (
    echo FOUND
    echo   Running nfCore tests in Win32/Release
    Bin\Win32\Release\nfCoreTest.exe
) else (
    echo NOT FOUND
)

echo.
echo | set /p=Checking if tests are compiled for x64/Debug... 
if exist Bin\x64\Debug\nfCoreTest.exe (
    echo FOUND
    echo   Running nfCore tests in x64/Debug
    Bin\x64\Debug\nfCoreTest.exe
) else (
    echo NOT FOUND
)

echo.
echo | set /p=Checking if tests are compiled for x64/Release... 
if exist Bin\x64\Release\nfCoreTest.exe (
    echo FOUND
    echo   Running nfCore tests in x64/Release
    Bin\x64\Release\nfCoreTest.exe
) else (
    echo NOT FOUND
)

popd
