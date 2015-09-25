/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Texture.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"

#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    mBindings.reserve(desc.numBindings);

    // TODO verify
    for (size_t i = 0; i < desc.numBindings; ++i)
        mBindings.push_back(desc.resourceBindings[i]);

    mShaderVisibility = desc.shaderVisibility;

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

bool ResourceBindingInstance::WriteTextureView(size_t slot, ITexture* texture, ISampler* sampler)
{
    UNUSED(slot);
    UNUSED(texture);
    UNUSED(sampler);
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
