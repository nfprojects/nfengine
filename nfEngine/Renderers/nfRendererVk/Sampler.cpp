/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan renderer's sampler
 */

#include "PCH.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

Sampler::~Sampler()
{
}

bool Sampler::Init(const SamplerDesc& desc)
{
    UNUSED(desc);
    LOG_INFO("Sampler created successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
