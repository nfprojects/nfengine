#!/bin/bash

pushd .

# remove Visual Studio files
rm -f Engine.sdf *.vsp *.psess

rm -rf ipch
rm -rf Bin
rm -rf Obj
rm -rf ShaderCache
rm -rf ShaderCache_Debug

popd
