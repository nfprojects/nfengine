@echo off

pushd .

set script_dir=%~dp0
cd "%script_dir%"
echo Current directory is %cd%

if NOT EXIST zlib\zconf.h (
    echo Moving zconf.h.included to zconf.h so Git is happy about Deps/zlib being clean
    ren "%script_dir%\zlib\zconf.h.included" zconf.h
)

popd

echo Script is done
