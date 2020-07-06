#include "PCH.hpp"
#include "PsoManager.hpp"
#include "ShaderManager.hpp"
#include "../../Renderers/RendererCommon/PipelineState.hpp"
#include "../../Renderers/RendererCommon/ComputePipelineState.hpp"

namespace NFE {
namespace Renderer {

PsoManager::PsoManager()
{

}

PipelineStatePtr PsoManager::AcquireGraphicsPSO(const Common::StringView shaderName, const PipelineStateDesc& stateDesc)
{
    NFE_UNUSED(shaderName);
    NFE_UNUSED(stateDesc);

    return nullptr;
}

ComputePipelineStatePtr PsoManager::AcquireComputePSO(const Common::StringView shaderName, const ComputePipelineStateDesc& stateDesc)
{
    NFE_UNUSED(shaderName);
    NFE_UNUSED(stateDesc);

    return nullptr;
}

} // namespace Scene
} // namespace NFE
