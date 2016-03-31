/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's renderer's shader resource binding
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
    mBindingSets.reserve(desc.numBindingSets);

    for (size_t i = 0; i < desc.numBindingSets; ++i)
    {
        ResourceBindingSet* bindingSet = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i]);
        if (bindingSet == nullptr)
        {
            LOG_ERROR("Invalid binding set");
            return false;
        }
        mBindingSets.push_back(bindingSet);
    }

    return true;
}

bool ResourceBindingInstance::Init(IResourceBindingSet* bindingSet)
{
    mBindingSet = dynamic_cast<ResourceBindingSet*>(bindingSet);
    if (bindingSet == nullptr)
    {
        LOG_ERROR("Invalid binding set");
        return false;
    }

    mViews.resize(mBindingSet->mBindings.size());

    return true;
}

bool ResourceBindingInstance::WriteTexture(size_t slot, ITexture* texture)
{
    if (slot >= mBindingSet->mBindings.size())
    {
        LOG_ERROR("Invalid binding set slot %zu (there are %zu slots)",
                  slot, mBindingSet->mBindings.size());
        return false;
    }

    Texture* tex = dynamic_cast<Texture*>(texture);
    if (!tex || !tex->mSRV)
    {
        LOG_ERROR("Invalid texture");
        return false;
    }

    mViews[slot] = tex->mSRV.get();
    return true;
}

bool ResourceBindingInstance::WriteCBuffer(size_t slot, IBuffer* buffer)
{
    if (slot >= mBindingSet->mBindings.size())
    {
        LOG_ERROR("Invalid binding set slot %zu (there are %zu slots)",
                  slot, mBindingSet->mBindings.size());
        return false;
    }

    Buffer* buf = dynamic_cast<Buffer*>(buffer);
    if (!buf || !buf->mBuffer)
    {
        LOG_ERROR("Invalid constant buffer");
        return false;
    }

    mViews[slot] = buf->mBuffer.get();
    return true;
}

bool ResourceBindingInstance::WriteSampler(size_t slot, ISampler* sampler)
{
    if (slot >= mBindingSet->mBindings.size())
    {
        LOG_ERROR("Invalid binding set slot %zu (there are %zu slots)",
                  slot, mBindingSet->mBindings.size());
        return false;
    }

    Sampler* samp = dynamic_cast<Sampler*>(sampler);
    if (!samp || !samp->mSamplerState)
    {
        LOG_ERROR("Invalid sampler");
        return false;
    }

    mViews[slot] = samp->mSamplerState.get();
    return true;
}

} // namespace Renderer
} // namespace NFE
