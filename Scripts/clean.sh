#!/bin/bash

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

# remove Visual Studio files
echo -n "Removing VS-related files... "
rm -f Engine.sdf *.vsp *.psess
rm -rf ipch
echo "DONE"

echo -n "Removing compilation results... "
rm -rf Bin
rm -rf Obj
echo "DONE"

echo -n "Removing shader compilation results... "
rm -rf ShaderCache
rm -rf ShaderCache_Debug
echo "DONE"

echo -n "Removing build-produced files... "
rm -rf build-*
echo "DONE"

echo -n "Removing other files... "
rm -f cppcheck_result.txt
rm -f *.pyc
echo "DONE"

popd > /dev/null
