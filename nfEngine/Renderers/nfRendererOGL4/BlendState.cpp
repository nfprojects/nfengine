/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OGL4 Renderer's Blend State definitions
 */

#include "PCH.hpp"
#include "PipelineState.hpp"

namespace NFE {
namespace Renderer {

BlendState::BlendState()
{
}

BlendState::~BlendState()
{
}

bool BlendState::Init(const BlendStateDesc& desc)
{
    // OGL states are split between functions
    // Remember whatever we must set, it will be done by CommandBuffer
    mDesc = desc;
    return true;
}

} // namespace Renderer
} // namespace NFE
