/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan renderer's sampler
 */

#include "PCH.hpp"
#include "Sampler.hpp"
#include "Device.hpp"

#include "Internal/Translations.hpp"


namespace NFE {
namespace Renderer {

Sampler::Sampler()
    : mSampler(VK_NULL_HANDLE)
{
}

Sampler::~Sampler()
{
    if (mSampler != VK_NULL_HANDLE)
        vkDestroySampler(gDevice->GetDevice(), mSampler, nullptr);
}

bool Sampler::Init(const SamplerDesc& desc)
{
    VkSamplerCreateInfo sampInfo;
    VK_ZERO_MEMORY(sampInfo);
    sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampInfo.magFilter = TranslateMagFilterToVkFilter(desc.magFilter);
    sampInfo.minFilter = TranslateMinFilterToVkFilter(desc.minFilter);
    sampInfo.mipmapMode = TranslateMinFilterToVkSamplerMipmapMode(desc.minFilter);
    sampInfo.addressModeU = TranslateWrapModeToVkSamplerAddressMode(desc.wrapModeU);
    sampInfo.addressModeV = TranslateWrapModeToVkSamplerAddressMode(desc.wrapModeV);
    sampInfo.addressModeW = TranslateWrapModeToVkSamplerAddressMode(desc.wrapModeW);
    sampInfo.mipLodBias = desc.mipmapBias;
    sampInfo.anisotropyEnable = (desc.maxAnisotropy > 1);
    sampInfo.maxAnisotropy = static_cast<float>(desc.maxAnisotropy);
    sampInfo.compareEnable = desc.compare;
    sampInfo.compareOp = TranslateCompareFuncToVkCompareOp(desc.compareFunc);
    sampInfo.minLod = desc.minMipmap;
    sampInfo.maxLod = desc.maxMipmap;
    sampInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    sampInfo.unnormalizedCoordinates = VK_FALSE;
    VkResult result = vkCreateSampler(gDevice->GetDevice(), &sampInfo, nullptr, &mSampler);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create Sampler");

    NFE_LOG_INFO("Sampler created successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
