#pragma once

#include "../RendererCommon/Types.hpp"
#include "D3DPtr.hpp"


namespace NFE {
namespace Renderer {

class ShaderCompiler
{
    D3DPtr<IDxcLibrary> mLibrary;
    D3DPtr<IDxcCompiler2> mCompiler;
    D3DPtr<IDxcIncludeHandler> mIncluder;

public:
    ShaderCompiler();
    ~ShaderCompiler() = default;

    bool Init();
    bool Compile(const char* source, uint32 sourceSize, const char* sourceName, ShaderType type,
                 const Common::DynArray<DxcDefine>& defines, D3DPtr<IDxcBlob>& output);
    bool Disassemble(const D3DPtr<IDxcBlob>& bytecode, D3DPtr<IDxcBlobEncoding>& disassembly);
    bool Reflect(const D3DPtr<IDxcBlob>& bytecode, D3DPtr<ID3D12ShaderReflection>& reflection);
};

} // namespace Renderer
} // namespace NFE
