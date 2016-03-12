/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OGL4 Renderer's pipeline state definitions
 */

#include "PCH.hpp"
#include "PipelineState.hpp"

namespace NFE {
namespace Renderer {

bool PipelineState::Init(const PipelineStateDesc& desc)
{
    // OGL states are split between functions
    // Remember whatever we must set, it will be done by CommandBuffer
    mDesc = desc;
    return true;
}

} // namespace Renderer
} // namespace NFE
