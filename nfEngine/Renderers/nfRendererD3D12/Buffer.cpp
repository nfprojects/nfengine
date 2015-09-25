/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's buffer
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "RendererD3D12.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

Buffer::Buffer()
{
}

bool Buffer::Init(const BufferDesc& desc)
{
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
