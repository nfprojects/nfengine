/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Texture.hpp"
#include "Device.hpp"

#include "Internal/Translations.hpp"
#include "Internal/Debugger.hpp"
#include "Internal/Utilities.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
    : mType(TextureType::Unknown)
    , mWidth(1)
    , mHeight(1)
    , mDepth(1)
    , mFormat(VK_FORMAT_UNDEFINED)
    , mImage(VK_NULL_HANDLE)
    , mImageView(VK_NULL_HANDLE)
    , mImageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
    , mImageLayoutDefault(VK_IMAGE_LAYOUT_UNDEFINED)
    , mImageMemory(VK_NULL_HANDLE)
    , mImageSubresRange()
{
}

Texture::~Texture()
{
    if (mImageView != VK_NULL_HANDLE)
        vkDestroyImageView(gDevice->GetDevice(), mImageView, nullptr);
    if (mImage != VK_NULL_HANDLE)
        vkDestroyImage(gDevice->GetDevice(), mImage, nullptr);
    if (mImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(gDevice->GetDevice(), mImageMemory, nullptr);
}

bool Texture::Init(const TextureDesc& desc)
{
    // TODO
    if (desc.type == TextureType::TextureCube)
    {
        NFE_LOG_ERROR("Cube textures unsupported");
        return false;
    }

    mType = desc.type;
    mFormat = TranslateFormatToVkFormat(desc.format);

    VkResult result = VK_SUCCESS;

    // Image creation
    VkImageCreateInfo imageInfo;
    VK_ZERO_MEMORY(imageInfo);
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    if (mType == TextureType::TextureCube)
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    switch (mType)
    {
    case TextureType::Texture1D:
        imageInfo.imageType = VK_IMAGE_TYPE_1D;
        break;
    case TextureType::Texture2D:
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        break;
    case TextureType::Texture3D:
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        break;
    case TextureType::TextureCube:
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        break;
    default:
        NFE_LOG_ERROR("Unsupported or incorrect texture type.");
        return false;
    }

    mWidth = desc.width;
    if (mType == TextureType::Texture2D || mType == TextureType::Texture3D || mType == TextureType::TextureCube)
        mHeight = desc.height;
    if (mType == TextureType::Texture3D)
        mDepth = desc.depth;

    VK_ZERO_MEMORY(mImageSubresRange);
    mImageSubresRange.baseArrayLayer = 0;
    mImageSubresRange.baseMipLevel = 0;
    mImageSubresRange.levelCount = desc.mipmaps;
    mImageSubresRange.layerCount = desc.layers;
    if (Util::IsDepthFormat(mFormat)) {
        mImageSubresRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (Util::FormatHasStencil(mFormat))
            mImageSubresRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else if (desc.format != Format::Unknown)
        mImageSubresRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;

    imageInfo.format = mFormat;
    imageInfo.extent.width = mWidth;
    imageInfo.extent.height = mHeight;
    imageInfo.extent.depth = mDepth;
    imageInfo.mipLevels = desc.mipmaps;
    imageInfo.arrayLayers = desc.layers;
    imageInfo.samples = TranslateSamplesNumToVkSampleCount(desc.samplesNum);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    if ((desc.usage & TextureUsageFlag::DepthStencil) == TextureUsageFlag::DepthStencil)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        mImageLayoutDefault = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if ((desc.usage & TextureUsageFlag::RenderTarget) == TextureUsageFlag::RenderTarget)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        mImageLayoutDefault = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if ((desc.usage & TextureUsageFlag::ReadonlyShaderResource) == TextureUsageFlag::ReadonlyShaderResource)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        mImageLayoutDefault = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    if ((desc.usage & TextureUsageFlag::ShaderWritableResource) == TextureUsageFlag::ShaderWritableResource)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        mImageLayoutDefault = VK_IMAGE_LAYOUT_GENERAL;
    }

    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    result = vkCreateImage(gDevice->GetDevice(), &imageInfo, nullptr, &mImage);
    CHECK_VKRESULT(result, "Failed to create Image for texture");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mImage), VK_OBJECT_TYPE_IMAGE, desc.debugName);

    VkMemoryRequirements imageMemReqs;
    vkGetImageMemoryRequirements(gDevice->GetDevice(), mImage, &imageMemReqs);

    VkMemoryAllocateInfo imageMemInfo;
    VK_ZERO_MEMORY(imageMemInfo);
    imageMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageMemInfo.allocationSize = imageMemReqs.size;
    imageMemInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(imageMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkAllocateMemory(gDevice->GetDevice(), &imageMemInfo, nullptr, &mImageMemory);
    CHECK_VKRESULT(result, "Failed to allocate memory for Image");

    if (desc.debugName)
    {
        Common::String memName(desc.debugName);
        memName += "-DeviceMemory";
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mImageMemory), VK_OBJECT_TYPE_DEVICE_MEMORY, memName.Str());
    }

    result = vkBindImageMemory(gDevice->GetDevice(), mImage, mImageMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind Image to its memory");


    VkImageViewCreateInfo viewInfo;
    VK_ZERO_MEMORY(viewInfo);
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = mImage;
    viewInfo.viewType = Util::TextureTypeToVkImageViewType(mType);
    viewInfo.format = mFormat;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewInfo.subresourceRange = mImageSubresRange;
    result = vkCreateImageView(gDevice->GetDevice(), &viewInfo, nullptr, &mImageView);
    CHECK_VKRESULT(result, "Failed to create image view for color attachment");

    mID = gDevice->GetLayoutTracker().Register(mImage, mImageSubresRange);

    NFE_LOG_INFO("Texture initialized successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
