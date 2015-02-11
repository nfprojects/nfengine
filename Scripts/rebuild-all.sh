#!/bin/bash

CPU_COUNT=`grep -c ^processor /proc/cpuinfo`
echo "Available CPU cores: ${CPU_COUNT}"
echo

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

Scripts/clean.sh
echo

# Define build platforms
declare -a BUILD_PLATFORMS=(i386 x86_64)

# For each platform...
for PLATFORM in ${BUILD_PLATFORMS[@]}; do
    # Clean cmake
    echo "Cleaning CMake files"
    Scripts/clean-cmake.sh
    echo

    # Build Debug
    echo "Building nfEngineDeps: Type Debug, Platform ${PLATFORM}"
    cmake . -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_BUILD_PLATFORM=${PLATFORM}
    make -j ${CPU_COUNT}
    echo

    # Clean cmake
    echo "Cleaning CMake files"
    Scripts/clean-cmake.sh
    echo

    # Build Release
    echo "Building nfEngineDeps: Type Release, Platform ${PLATFORM}"
    cmake . -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_BUILD_PLATFORM=${PLATFORM}
    make -j ${CPU_COUNT}
    echo
done

popd > /dev/null
