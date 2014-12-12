#!/bin/bash

echo -n "Checking for cppcheck... "
EXE_PATH=$(which cppcheck)
if [ ! -x "$EXE_PATH" ] ; then
    echo "NOT FOUND"
    echo "    Make sure you download cppcheck before using this script."
    echo "    See README.md for more info. Exiting."
    read -p "Press [Enter] to continue. "
    exit
else
    echo "FOUND"
fi

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

cppcheck ./nfEngine/ -j 8 --enable=warning 2> cppcheck_result.txt

popd > /dev/null
