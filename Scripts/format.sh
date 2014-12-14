#!/bin/bash

echo -n "Checking for astyle... "
EXE_PATH=$(which astyle)
if [ ! -x "$EXE_PATH" ] ; then
    echo "NOT FOUND"
    echo "    Make sure you download astyle before using this script."
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

if [ "$1" == "dry" ] ; then
    # launch astyle for all projects
    astyle --options=Scripts/style.cfg --dry-run --formatted -r -v --suffix=none --exclude=nfEngine/nfCore/Prerequisites.hpp nfEngine/*.cpp nfEngine/*.hpp
    echo
    # Prerequisites.hpp should have namespaces indented to improve readability
    # run astyle once more specifically on this file
    astyle --options=Scripts/style.cfg --dry-run --formatted -v --suffix=none --indent-namespaces nfEngine/nfCore/Prerequisites.hpp
else
    # launch astyle for all projects
    astyle --options=Scripts/style.cfg -r -v --suffix=none --exclude=nfEngine/nfCore/Prerequisites.hpp nfEngine/*.cpp nfEngine/*.hpp
    echo
    # Prerequisites.hpp should have namespaces indented to improve readability
    # run astyle once more specifically on this file
    astyle --options=Scripts/style.cfg -v --suffix=none --indent-namespaces nfEngine/nfCore/Prerequisites.hpp
fi

popd > /dev/null
