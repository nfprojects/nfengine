#include "PCH.hpp"
#include "ShaderManager.hpp"
#include "../../Renderers/RendererCommon/Shader.hpp"
#include "../../Renderers/RendererCommon/PipelineState.hpp"
#include "../../Renderers/RendererCommon/ComputePipelineState.hpp"

namespace NFE {
namespace Renderer {

ShaderManager::ShaderManager()
{

}

ShaderManager::~ShaderManager()
{

}

GraphicsShader ShaderManager::AcquireGraphicsShader(const Common::StringView shaderName)
{
    return {};
}

ShaderPtr ShaderManager::AcquireComputeShader(const Common::StringView shaderName)
{
    return nullptr;
}


void ShaderManager::RegisterGraphicsShader(const Common::StringView shaderName, const GraphicsShaderDesc& desc)
{

}

} // namespace Scene
} // namespace NFE
