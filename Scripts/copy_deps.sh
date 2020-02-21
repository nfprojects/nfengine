#!/bin/bash

# WARNING
# This script was ported in a "just in case" matter.
# It was not heavily tested, as we still don't know if we'll ever use it.

# script assumes following arguments:
#   %1   - PlatformDirName
#   %2   - DestConfigurationDirName
#   %3   - SrcConfigurationDirName
#   %4.. - <list of files to copy>

# verify that some arguments were given
if [ -z "$4" ] ; then
    echo "ERROR: There were not enough arguments given. Was that intentional?"
    echo "       Leaving script."
    exit
fi

# go to base nfEngine dir
pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

# verify if deps are downloaded and compiled
if [ ! -d "nfEngineDeps/Bin" ] ; then
    echo "ERROR: nfEngineDeps is not downloaded/compiled."
    echo "       Please, download and compile nfEngineDeps first. See README.md for more info."
    popd > /dev/null
    exit
fi

BasePath="$PWD/nfEngineDeps/Bin/$1/$3/"
DestPath="$PWD/Bin/$1/$2/"
echo "Deps files will be copied from $BasePath to $DestPath"

while :; do
if [ -z "$4" ] ; then
    break
fi

if [ -x "$BasePath$4" ] ; then
    echo "Copying $4"
    cp "$BasePath$4" "$DestPath$4"
else
    echo "WARN:  $4 not found - skipping..."
fi

shift

done

echo Finished
popd > /dev/null
