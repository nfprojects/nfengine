@echo off

pushd .
cd %~dp0\..

(echo #pragma once) > SHA1.hpp
(echo|set /p=const char* gCommitHash = ^") >> SHA1.hpp
type .git\HEAD >> SHA1.hpp
(echo ^";) >> SHA1.hpp

popd
