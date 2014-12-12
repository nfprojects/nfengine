#!/bin/bash

# WARNING
# This script was ported in a "just in case" matter.
# It was not heavily tested, as we still don't know if we'll ever use it.

# script assumes following arguments:
#   %1   - PlatformDirName
#   %2   - ConfigurationDirName
#   %3.. - <list of files to copy>

# verify that some arguments were given
if [ -z "$3" ] ; then
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

BasePath="$PWD/nfEngineDeps/Bin/$1/$2/"
DestPath="$PWD/Bin/$1/$2/"
echo "Files will be copied to $DestPath"

while :; do
if [ -z "$3" ] ; then
    break
fi

if [ -x "$BasePath$3" ] ; then
    echo "Copying $3"
    cp "$BasePath$3" "$DestPath$3"
else
    echo "WARN:  $3 not found - skipping..."
fi

shift

done

echo Finished
popd > /dev/null
