/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Multishader utility
 */

#pragma once

#include "Renderer.hpp"
#include "../nfCommon/Buffer.hpp"

namespace NFE {
namespace Renderer {

/*

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

class Multishader
{
private:
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
    bool Init(ShaderType type, const char* shaderName);
    void Release();
    ShaderType GetType() const;

    Common::Buffer* GetShaderBytecode(UINT* pMacroValues);
};

*/

} // namespace Renderer
} // namespace NFE
