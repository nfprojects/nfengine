/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 12 render's pipeline states.
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    friend class CommandBuffer;
public:
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
