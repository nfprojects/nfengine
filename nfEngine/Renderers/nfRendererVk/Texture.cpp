/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Texture.hpp"
#include "Translations.hpp"
#include "Device.hpp"


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
    , mImageMemory(VK_NULL_HANDLE)
    , mBuffersNum(0)
    , mCurrentBuffer(0)
    , mFromSwapchain(false)
{
}

Texture::~Texture()
{
    if (mImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(gDevice->GetDevice(), mImageMemory, nullptr);
    if (mImageView != VK_NULL_HANDLE)
        vkDestroyImageView(gDevice->GetDevice(), mImageView, nullptr);
    if (mImage != VK_NULL_HANDLE)
        vkDestroyImage(gDevice->GetDevice(), mImage, nullptr);


    for (auto& buf : mBuffers)
        if (!mFromSwapchain)
            if (buf != VK_NULL_HANDLE)
                vkDestroyImage(gDevice->GetDevice(), buf, nullptr);

    for (auto& bufview : mBufferViews)
        if (bufview != VK_NULL_HANDLE)
            vkDestroyImageView(gDevice->GetDevice(), bufview, nullptr);
}

bool Texture::Init(const TextureDesc& desc)
{
    // TODO
    if (desc.type == TextureType::TextureCube)
    {
        LOG_ERROR("Cube textures unsupported");
        return false;
    }

    mType = desc.type;
    if (desc.depthBufferFormat != DepthBufferFormat::Unknown)
        mFormat = TranslateDepthFormatToVkFormat(desc.depthBufferFormat);
    else
        mFormat = TranslateElementFormatToVkFormat(desc.format);

    VkResult result = VK_SUCCESS;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    if (desc.dataDesc)
    {
        // create buffer to store texture data
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
            LOG_ERROR("Unsupported or incorrect texture type.");
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
        LOG_ERROR("Unsupported or incorrect texture type.");
        return false;
    }

    mWidth = desc.width;
    if (mType == TextureType::Texture2D || mType == TextureType::Texture3D || mType == TextureType::TextureCube)
        mHeight = desc.height;
    if (mType == TextureType::Texture3D)
        mDepth = desc.depth;

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
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_RENDERTARGET)
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
        imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    result = vkCreateImage(gDevice->GetDevice(), &imageInfo, nullptr, &mImage);
    CHECK_VKRESULT(result, "Failed to create Image for texture");

    VkMemoryRequirements imageMemReqs;
    vkGetImageMemoryRequirements(gDevice->GetDevice(), mImage, &imageMemReqs);

    VkMemoryAllocateInfo imageMemInfo;
    VK_ZERO_MEMORY(imageMemInfo);
    imageMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageMemInfo.allocationSize = imageMemReqs.size;
    imageMemInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(imageMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkAllocateMemory(gDevice->GetDevice(), &imageMemInfo, nullptr, &mImageMemory);
    CHECK_VKRESULT(result, "Failed to allocate memory for Image");

    result = vkBindImageMemory(gDevice->GetDevice(), mImage, mImageMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind Image to its memory");

    VkImageSubresourceRange imgSubresourceRange;
    VK_ZERO_MEMORY(imgSubresourceRange);
    if (desc.depthBufferFormat != DepthBufferFormat::Unknown)
    {
        if (desc.depthBufferFormat == DepthBufferFormat::Depth24_Stencil8)
            imgSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        else 
            imgSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
        imgSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgSubresourceRange.baseMipLevel = 0;
    imgSubresourceRange.levelCount = desc.mipmaps;
    imgSubresourceRange.layerCount = desc.layers;

    if (desc.dataDesc)
    {
        // TODO right now copying is done on a general queue, but the devices support separate Transfer queue.
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
        imageBarrier.image = mImage;
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier.subresourceRange = imgSubresourceRange;
        vkCmdPipelineBarrier(copyCmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

        // TODO Multiple dataDescs support
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

        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(copyCmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

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
        vkFreeMemory(gDevice->GetDevice(), stagingBufferMemory, nullptr);
        vkDestroyBuffer(gDevice->GetDevice(), stagingBuffer, nullptr);
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
        LOG_ERROR("Unsupported or incorrect texture type.");
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
    ivInfo.subresourceRange = imgSubresourceRange;
    result = vkCreateImageView(gDevice->GetDevice(), &ivInfo, nullptr, &mImageView);
    CHECK_VKRESULT(result, "Failed to generate Image View from created Texure's image");

    LOG_INFO("Texture initialized successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
