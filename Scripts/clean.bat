pushd .
cd ..

del /F /Q "Engine.sdf"
del /F /Q "*.vsp"
del /F /Q "*.psess"

rd /S /Q "ipch"
rd /S /Q "Bin"
rd /S /Q "Obj"

rd /S /Q "ShaderCache"
rd /S /Q "ShaderCache_Debug"

popd
