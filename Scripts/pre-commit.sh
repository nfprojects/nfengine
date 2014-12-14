#!/bin/bash

echo
echo "Pre-commit script for nfEngine project"
echo "    Script meant to verify if provided commit is OK"
echo.

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}

echo "====================="
echo "Running code analysis"
echo "====================="
echo

cppcheck.sh quiet
if [ -f "../cppcheck_result.txt" ] ; then
    echo "Results from cppcheck:"
    cat ../cppcheck_result.txt
fi

echo
echo "==================="
echo "Running style check"
echo "==================="
echo

format.sh dry

echo
echo "============="
echo "Running tests"
echo "============="
echo

tests.sh

popd > /dev/null

echo
echo "    Pre-commit script has finished its work."
echo "    Remember to compare these results with current top commit on devel branch."
echo
echo "    If you see any errors, fix them before committing. Otherwise, failed"
echo "    verification during review is imminent."
echo
