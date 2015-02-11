#!/bin/bash

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

echo "Searching and cleaning only CMake-related files"

pushd . > /dev/null
cd nfEngine

find . -name CMakeFiles | xargs rm -rf
find . -name CMakeCache.txt | xargs rm -f
find . -name cmake_install.cmake | xargs rm -f
find . -name BulletConfig.cmake | xargs rm -f
find . -name \*.pc | xargs rm -f

popd > /dev/null

popd > /dev/null
