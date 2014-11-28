#!/bin/bash

# Check for astyle
echo -n "Checking for astyle... "
type astyle >/dev/null 2>&1 || {
    echo "NOT FOUND";
    echo "    Make sure you download astyle before using this script."
    echo "    See README.md for more info. Exiting."
    exit 1
}
echo FOUND
echo Current launch directory is pwd


#astyle --options=style.cfg -r -v --suffix=none ../nfEngine/*.cpp ../nfEngine/*.h ../nfEngineTest/*.cpp ../nfEngineTest/*.h
