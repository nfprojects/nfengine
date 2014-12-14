#!/bin/bash

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd


# nfCommon tests
echo
echo "===== Running nfCommon tests ====="
echo
echo -n "Checking if tests are compiled for Win32/Debug... "
if [ -x "Bin/Win32/Debug/nfCommonTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCommon tests in Win32/Debug"
    Bin/Win32/Debug/nfCommonTest.exe
else
    echo NOT FOUND
fi

echo
echo -n "Checking if tests are compiled for Win32/Release... "
if [ -x "Bin/Win32/Release/nfCommonTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCommon tests in Win32/Release"
    Bin/Win32/Release/nfCommonTest.exe
else
    echo NOT FOUND
fi

echo
echo -n "Checking if tests are compiled for x64/Debug... "
if [ -x "Bin/x64/Debug/nfCommonTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCommon tests in x64/Debug"
    Bin/x64/Debug/nfCommonTest.exe
else
    echo NOT FOUND
fi

echo
echo -n "Checking if tests are compiled for x64/Release... "
if [ -x "Bin/x64/Release/nfCommonTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCommon tests in x64/Release"
    Bin/x64/Release/nfCommonTest.exe
else
    echo NOT FOUND
fi

# nfCore tests
echo
echo "====== Running nfCore tests ======"
echo
echo -n "Checking if tests are compiled for Win32/Debug... "
if [ -x "Bin/Win32/Debug/nfCoreTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCore tests in Win32/Debug"
    Bin/Win32/Debug/nfCoreTest.exe
else
    echo NOT FOUND
fi

echo
echo -n "Checking if tests are compiled for Win32/Release... "
if [ -x "Bin/Win32/Release/nfCoreTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCore tests in Win32/Release"
    Bin/Win32/Release/nfCoreTest.exe
else
    echo NOT FOUND
fi

echo
echo -n "Checking if tests are compiled for x64/Debug... "
if [ -x "Bin/x64/Debug/nfCoreTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCore tests in x64/Debug"
    Bin/x64/Debug/nfCoreTest.exe
else
    echo NOT FOUND
fi

echo
echo -n "Checking if tests are compiled for x64/Release... "
if [ -x "Bin/x64/Release/nfCoreTest.exe" ] ; then
    echo "FOUND"
    echo "  Running nfCore tests in x64/Release"
    Bin/x64/Release/nfCoreTest.exe
else
    echo NOT FOUND
fi

popd > /dev/null
