#!/bin/bash

cd ..
cppcheck ./nfEngine/ -j 8 --enable=warning 2> Scripts/cppcheck_result.txt
cd -
