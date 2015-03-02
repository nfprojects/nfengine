#pragma once
#include "nfRenderer.hpp"
#include "../nfCommon/Buffer.hpp"

namespace NFE {
namespace Render {

class RendererD3D11;

enum class ShaderType
{
    Unknown = 0,
    Vertex,
    Geometry,
    Pixel,
    Domain,
    Hull,
    Compute,
};

// maximum number of macro definitions added to one shader
#define X_MULTISHADER_MAX_MACROS 32

class SubshaderInfo
{
public:
    Common::Buffer shaderCode;
    void* shader;
    UINT hash;

    SubshaderInfo() : shader(0), hash(0) {};
};

class MultshaderMacro
{
public:
    UINT maxValue;
    UINT bits;
    char name[24];

};

// for now, it supports only Direct3D 11 shaders
class Multishader
{
private:
    RendererD3D11* mRenderer;

    bool CreateSubshader(UINT hash, Common::Buffer& buffer);
    bool LoadSubshader(UINT* pMacros, std::string& CompileLog);

    UINT mCurrHash;
    std::string mShaderName;
    ShaderType mType;

public:

    std::map<UINT, SubshaderInfo> shaders;
    std::vector<MultshaderMacro> macros;

    Multishader();
    ~Multishader();

    void AddMacro(const char* pName, UINT MaxValue, UINT BitOffset);
    bool Init(RendererD3D11* pRenderer, ShaderType type, const char* shaderName);
    void Release();
    ShaderType GetType() const;

    Common::Buffer* GetShaderBytecode(UINT* pMacroValues);
};

} // namespace Render
} // namespace NFE
