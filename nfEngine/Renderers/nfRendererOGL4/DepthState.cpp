/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OGL4 Renderer's Depth State definitions
 */

#include "PCH.hpp"
#include "PipelineState.hpp"

namespace NFE {
namespace Renderer {

DepthState::DepthState()
{
}

DepthState::~DepthState()
{
}

bool DepthState::Init(const DepthStateDesc& desc)
{
    // OGL states are split between functions
    // Remember whatever we must set, it will be done by CommandBuffer
    mDesc = desc;
    return true;
}

} // namespace Renderer
} // namepsace NFE
