@echo off

pushd .
cd %~dp0

echo.
echo Pre-commit script for nfEngine project
echo     Script meant to verify if provided commit is OK
echo.
echo =====================
echo Running code analysis
echo =====================
echo.

call cppcheck.bat quiet
if exist ..\cppcheck_result.txt (
    echo Results from cppcheck:
    type ..\cppcheck_result.txt
)

echo.
echo ===================
echo Running style check
echo ===================
echo.

call format.bat dry

echo.
echo =============
echo Running tests
echo =============
echo.

call tests.bat

popd

echo.
echo     Pre-commit script has finished its work.
echo     Remember to compare these results with current top commit on devel branch.
echo.
echo     If you see any errors, fix them before committing. Otherwise, failed
echo     verification during review is imminent.
echo.
