/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Texture.hpp"
#include "Device.hpp"

#include "Internal/Translations.hpp"
#include "Internal/Debugger.hpp"


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
    , mImageMemory(VK_NULL_HANDLE)
    , mImageSubresRange()
{
}

Texture::~Texture()
{
    // TODO this needs to go away
    gDevice->WaitForGPU();

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

    bool hasInitialData = (desc.dataDesc != nullptr);

    mType = desc.type;
    if (desc.depthBufferFormat != DepthBufferFormat::Unknown)
        mFormat = TranslateDepthFormatToVkFormat(desc.depthBufferFormat);
    else
        mFormat = TranslateElementFormatToVkFormat(desc.format);

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
    if (desc.depthBufferFormat != DepthBufferFormat::Unknown)
    {
        mImageSubresRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (desc.depthBufferFormat == DepthBufferFormat::Depth24_Stencil8)
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

    // TODO SHADER_WRITABLE
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        mImageLayoutDefault = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_RENDERTARGET)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        mImageLayoutDefault = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        mImageLayoutDefault = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER_WRITABLE)
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

    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    if (hasInitialData)
    {
        // create a staging buffer to store texture data
        VkBufferCreateInfo bufInfo;
        VK_ZERO_MEMORY(bufInfo);
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // TODO multiple dataDesc support
        switch (mType)
        {
        case TextureType::Texture1D:
            bufInfo.size = desc.dataDesc[0].lineSize;
            break;
        case TextureType::Texture2D:
            bufInfo.size = desc.dataDesc[0].sliceSize;
            break;
        case TextureType::Texture3D:
            bufInfo.size = desc.dataDesc[0].sliceSize * desc.width;
            break;
        default:
            NFE_LOG_ERROR("Unsupported or incorrect texture type.");
            return false;
        }

        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &stagingBuffer);
        CHECK_VKRESULT(result, "Failed to create staging buffer for data upload");

        VkMemoryRequirements stagingMemReqs;
        vkGetBufferMemoryRequirements(gDevice->GetDevice(), stagingBuffer, &stagingMemReqs);

        VkMemoryAllocateInfo memInfo;
        VK_ZERO_MEMORY(memInfo);
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.allocationSize = stagingMemReqs.size;
        memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(stagingMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &stagingBufferMemory);
        CHECK_VKRESULT(result, "Failed to allocate memory for staging buffer");

        void* bufferData = nullptr;
        result = vkMapMemory(gDevice->GetDevice(), stagingBufferMemory, 0, memInfo.allocationSize, 0, &bufferData);
        CHECK_VKRESULT(result, "Failed to map staging buffer's memory");
        memcpy(bufferData, desc.dataDesc[0].data, static_cast<size_t>(bufInfo.size));
        vkUnmapMemory(gDevice->GetDevice(), stagingBufferMemory);
        result = vkBindBufferMemory(gDevice->GetDevice(), stagingBuffer, stagingBufferMemory, 0);
        CHECK_VKRESULT(result, "Failed to bind staging buffer to its memory");
    }

    // TODO right now copying is done on a general queue, but devices can support separate Transfer queue.
    //      Consider moving copy command buffers to transfer queue if device makes it possible.
    VkCommandBuffer copyCmdBuffer;
    VkCommandBufferAllocateInfo cmdInfo;
    VK_ZERO_MEMORY(cmdInfo);
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.commandPool = gDevice->GetCommandPool();
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = 1;
    result = vkAllocateCommandBuffers(gDevice->GetDevice(), &cmdInfo, &copyCmdBuffer);
    CHECK_VKRESULT(result, "Failed to allocate a command buffer");

    VkCommandBufferBeginInfo cmdBeginInfo;
    VK_ZERO_MEMORY(cmdBeginInfo);
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    result = vkBeginCommandBuffer(copyCmdBuffer, &cmdBeginInfo);
    CHECK_VKRESULT(result, "Failed to begin command rendering for buffer copy operation");

    VkImageMemoryBarrier imageBarrier;
    VK_ZERO_MEMORY(imageBarrier);
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.srcAccessMask = 0;
    imageBarrier.dstAccessMask = 0;
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier.image = mImage;
    imageBarrier.subresourceRange = mImageSubresRange;

    if (hasInitialData)
    {
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        // assume all barriers are full blocking, like in D3D12 renderer
        vkCmdPipelineBarrier(copyCmdBuffer,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &imageBarrier);

        VkBufferImageCopy imageCopyRegion;
        VK_ZERO_MEMORY(imageCopyRegion);
        imageCopyRegion.bufferOffset = 0;
        imageCopyRegion.imageExtent.width = mWidth;
        imageCopyRegion.imageExtent.height = mHeight;
        imageCopyRegion.imageExtent.depth = mDepth;
        imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.imageSubresource.layerCount = desc.layers;
        imageCopyRegion.imageSubresource.mipLevel = 0;
        vkCmdCopyBufferToImage(copyCmdBuffer, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
    }

    // TODO this is a 100% tempshit, migrate transitions to ResourceState
    imageBarrier.oldLayout = imageBarrier.newLayout;
    imageBarrier.newLayout = mImageLayoutDefault;
    mImageLayout = mImageLayoutDefault;

    // assume all barriers are full blocking, like in D3D12 renderer
    vkCmdPipelineBarrier(copyCmdBuffer,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &imageBarrier);

    result = vkEndCommandBuffer(copyCmdBuffer);
    CHECK_VKRESULT(result, "Failure during copy command buffer recording");

    VkSubmitInfo submitInfo;
    VK_ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCmdBuffer;
    vkQueueSubmit(gDevice->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    gDevice->WaitForGPU();

    vkFreeCommandBuffers(gDevice->GetDevice(), gDevice->GetCommandPool(), 1, &copyCmdBuffer);

    if (hasInitialData)
    {
        vkDestroyBuffer(gDevice->GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(gDevice->GetDevice(), stagingBufferMemory, nullptr);
    }

    VkImageViewCreateInfo ivInfo;
    VK_ZERO_MEMORY(ivInfo);
    ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivInfo.image = mImage;

    switch (mType)
    {
    case TextureType::Texture1D:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        break;
    case TextureType::Texture2D:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case TextureType::Texture3D:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case TextureType::TextureCube:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        break;
    default:
        NFE_LOG_ERROR("Unsupported or incorrect texture type.");
        return false;
    }

    ivInfo.format = imageInfo.format;
    ivInfo.components = {
            // order of variables in VkComponentMapping is r, g, b, a
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A,
    };
    ivInfo.subresourceRange = mImageSubresRange;
    result = vkCreateImageView(gDevice->GetDevice(), &ivInfo, nullptr, &mImageView);
    CHECK_VKRESULT(result, "Failed to generate Image View from created Texure's image");

    if (desc.debugName)
    {
        Common::String ivName(desc.debugName);
        ivName += "-View";
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mImageView), VK_OBJECT_TYPE_IMAGE_VIEW, ivName.Str());
    }

    NFE_LOG_INFO("Texture initialized successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
