@echo off

pushd .
cd %~dp0\..
echo Current directory is %cd%

echo | set /p=Removing VS-related files... 
del /F /Q "Engine.sdf" 2> clean_errors.txt
del /F /Q "*.vsp" 2>> clean_errors.txt
del /F /Q "*.psess" 2>> clean_errors.txt
rd /S /Q "ipch" 2>> clean_errors.txt
echo DONE

echo | set /p=Removing compilation results... 
rd /S /Q "Bin" 2>> clean_errors.txt
rd /S /Q "Obj" 2>> clean_errors.txt
echo DONE

echo | set /p=Removing shader compilation results... 
rd /S /Q "ShaderCache" 2>> clean_errors.txt
rd /S /Q "ShaderCache_Debug" 2>> clean_errors.txt
echo DONE

echo | set /p=Removing other files... 
del /F /Q "cppcheck_result.txt"
echo DONE

popd
