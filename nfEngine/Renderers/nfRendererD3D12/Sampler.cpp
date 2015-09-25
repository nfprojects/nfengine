/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture sampler
 */

#include "PCH.hpp"
#include "Sampler.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

bool Sampler::Init(const SamplerDesc& desc)
{
    UNUSED(desc);
    return true;
}

} // namespace Renderer
} // namespace NFE
