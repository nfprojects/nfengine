#!/bin/bash

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

# remove CMake files
echo -n "Removing CMake-related files... "
rm -rf CMakeCache.txt cmake_install.cmake Makefile
find . -type d -name CMakeFiles -prune -exec rm -rf {} \;
echo "DONE"

echo -n "Removing compilation results... "
rm -rf Bin
rm -rf Obj
echo "DONE"

echo -n "Removing build-produced files... "
rm -rf build-*
echo "DONE"

popd > /dev/null
