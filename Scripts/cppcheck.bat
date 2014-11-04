pushd .
cd %~dp0\..
echo Current dir is %cd%
cppcheck ./nfEngine/ -j 8 --enable=warning 2> Scripts/cppcheck_result.txt
popd
