#!/bin/bash

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

echo "Cleaning entire nfEngineDeps directory"
Scripts/clean-cmake.sh

echo "Removing compiled libraries"
rm -rf Bin

echo "Removing AntTweakBar compilation results"
pushd . > /dev/null
cd AntTweakBar

find . -name libAntTweakBar\* | xargs rm -f
find . -name \*.o | xargs rm -f

popd > /dev/null

popd > /dev/null
