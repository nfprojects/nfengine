PUSHD .
CD ..

DEL /F /Q "Engine.sdf"
DEL /F /Q "*.vsp"
DEL /F /Q "*.psess"

RD /S /Q "ipch"
RD /S /Q "Bin"
RD /S /Q "Obj"

RD /S /Q "ShaderCache"
RD /S /Q "ShaderCache_Debug"

POPD
