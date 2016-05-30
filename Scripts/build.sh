#!/bin/bash

CPU_COUNT=`grep -c ^processor /proc/cpuinfo`
echo "Available CPU cores: ${CPU_COUNT}"
echo

if [ "$#" -gt 1 ];
then
    echo "Incorrect number of parameters."
    echo "Usage: build.sh [BUILD_TYPE]"
    echo
    echo "Where:"
    echo "    BUILD_TYPE - type of build (debug/release)"
    echo
fi

if [ "$#" -eq 0 ];
then
    echo "No BUILD_TYPE argument - assuming Release build"
    BUILD_TYPE="Release"
else
    BUILD_TYPE=$1
fi


pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

echo

# Clean cmake
echo "Cleaning CMake files"
Scripts/clean-cmake.sh
echo

# Build with given configuration
cmake . -DCMAKE_BUILD_TYPE=$BUILD_TYPE
make -j ${CPU_COUNT}
echo

popd > /dev/null
