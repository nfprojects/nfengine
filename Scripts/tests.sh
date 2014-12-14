#!/bin/bash

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

# TODO: this must be done when nfCommon (and tests) will be ported to Linux
#       Reasons:
#         1. We don't know what Linux paths will be
#         2. This and Batch version might be totally replaced by Python script ( Issue #101 )
echo "WARNING: not implemented"

popd > /dev/null
