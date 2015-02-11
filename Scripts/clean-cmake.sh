#!/bin/bash

echo "Searching and cleaning only CMake-related files"
find . -name CMakeFiles | xargs rm -rf
find . -name CMakeCache.txt | xargs rm
find . -name cmake_install.cmake | xargs rm
find . -name \*.pc | xargs rm
