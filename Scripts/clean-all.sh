#!/bin/bash

echo "Cleaning entire nfEngineDeps directory"
./clean-cmake.sh

echo "Removing compiled libraries"
rm -rf Bin
