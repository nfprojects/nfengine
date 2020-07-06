#pragma once

#include "../RendererApi.hpp"
#include "../../Renderers/RendererCommon/Shader.hpp"
#include "../../Renderers/RendererCommon/PipelineState.hpp"
#include "../../Renderers/RendererCommon/ComputePipelineState.hpp"

namespace NFE {
namespace Renderer {

struct PipelineStateDesc;
struct ComputePipelineStateDesc;

/**
 * Class responsible for loading, caching and creating Pipeline State Objects.
 */
class PsoManager final
{
    NFE_MAKE_NONCOPYABLE(PsoManager)
    NFE_MAKE_NONMOVEABLE(PsoManager)

public:

    PsoManager();

    PipelineStatePtr AcquireGraphicsPSO(const Common::StringView shaderName, const PipelineStateDesc& stateDesc);
    ComputePipelineStatePtr AcquireComputePSO(const Common::StringView shaderName, const ComputePipelineStateDesc& stateDesc);

private:

    struct PsoDescPair
    {
        Common::StringView shaderName;
        PipelineStateDesc stateDesc;
    };
};


} // namespace Renderer
} // namespace NFE
