#!/bin/bash

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

Scripts/clean.sh
echo

Scripts/build.sh Debug
Scripts/build.sh Release

popd > /dev/null
