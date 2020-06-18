/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan renderer's sampler
 */

#include "PCH.hpp"
#include "Sampler.hpp"
#include "Device.hpp"

#include "Internal/Translations.hpp"
#include "Internal/Debugger.hpp"


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
    CHECK_VKRESULT(result, "Failed to create Sampler");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mSampler), VK_OBJECT_TYPE_SAMPLER, desc.debugName);

    NFE_LOG_INFO("Sampler created successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
