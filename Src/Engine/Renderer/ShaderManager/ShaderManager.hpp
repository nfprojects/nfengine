#pragma once

#include "../RendererApi.hpp"
#include "../../Renderers/RendererCommon/Shader.hpp"
#include "../../Renderers/RendererCommon/PipelineState.hpp"
#include "../../Renderers/RendererCommon/ComputePipelineState.hpp"

namespace NFE {
namespace Renderer {

struct PipelineStateDesc;
struct ComputePipelineStateDesc;

struct GraphicsShader
{
    ShaderPtr vertexShader;
    ShaderPtr hullShader;
    ShaderPtr domainShader;
    ShaderPtr geometryShader;
    ShaderPtr pixelShader;
};

/**
 * Class responsible for loading, caching and compiling shader groups.
 */
class ShaderManager final
{
    NFE_MAKE_NONCOPYABLE(ShaderManager)
    NFE_MAKE_NONMOVEABLE(ShaderManager)

public:

    ShaderManager();
    ~ShaderManager();

    GraphicsShader AcquireGraphicsShader(const Common::StringView shaderName);
    ShaderPtr AcquireComputeShader(const Common::StringView shaderName);

private:

    struct ShaderMacro
    {
        Common::String name;
        Common::String value;
    };

    struct GraphicsShaderDesc
    {
        Common::String vertexShaderPath;
        Common::String pixelShaderPath;
        Common::DynArray<ShaderMacro> macros;
    };

    struct SourceFileInfo
    {
        Common::String filePath;
        uint64 modificationDate;
        uint64 fileHash;
    };

    void RegisterGraphicsShader(const Common::StringView shaderName, const GraphicsShaderDesc& desc);
};


} // namespace Renderer
} // namespace NFE
