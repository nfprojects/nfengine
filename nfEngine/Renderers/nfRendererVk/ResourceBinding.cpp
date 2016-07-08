/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"


namespace NFE {
namespace Renderer {

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    UNUSED(desc);
    return true;
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    UNUSED(desc);
    return true;
}

bool ResourceBindingInstance::Init(IResourceBindingSet* bindingSet)
{
    UNUSED(bindingSet);
    return false;
}

bool ResourceBindingInstance::WriteTextureView(size_t slot, ITexture* texture)
{
    UNUSED(slot);
    UNUSED(texture);
    return false;
}

bool ResourceBindingInstance::WriteCBufferView(size_t slot, IBuffer* buffer)
{
    UNUSED(slot);
    UNUSED(buffer);
    return false;
}

} // namespace Renderer
} // namespace NFE
