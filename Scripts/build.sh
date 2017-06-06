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

# ensure case-sensitiveness
BUILD_TYPE="$(tr '[:lower:]' '[:upper:]' <<< ${BUILD_TYPE:0:1})${BUILD_TYPE:1}"

mkdir -p build-$BUILD_TYPE

# Build with given configuration
cd build-$BUILD_TYPE
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE
make -j ${CPU_COUNT}
rc=$?
echo

popd > /dev/null

exit $rc
