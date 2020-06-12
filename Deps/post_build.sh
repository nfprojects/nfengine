#!/bin/bash

pushd . > /dev/null
cd $(dirname "${BASH_SOURCE[0]}")
echo -n "Current deirectory is "; pwd


# move zconf.h.included back to zconf.h
if [ ! -e "./zlib/zconf.h" ]; then
    echo "Moving zconf.h.included to zconf.h so Git is happy about Deps/zlib being clean"
    mv ./zlib/zconf.h.included ./zlib/zconf.h
fi

popd > /dev/null

echo "Post-build script is done"
