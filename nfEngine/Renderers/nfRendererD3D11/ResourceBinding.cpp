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
    if (desc.numBindings == 0)
    {
        LOG_ERROR("Binding set can not be empty");
        return false;
    }

    if (desc.shaderVisibility != ShaderType::Vertex &&
        desc.shaderVisibility != ShaderType::Hull &&
        desc.shaderVisibility != ShaderType::Domain &&
        desc.shaderVisibility != ShaderType::Geometry &&
        desc.shaderVisibility != ShaderType::Pixel &&
        desc.shaderVisibility != ShaderType::All)
    {
        LOG_ERROR("Invalid shader visibility");
        return false;
    }


    mBindings.reserve(desc.numBindings);

    for (size_t i = 0; i < desc.numBindings; ++i)
    {
        const ResourceBindingDesc& bindingDesc = desc.resourceBindings[i];

        if (bindingDesc.resourceType != ShaderResourceType::CBuffer &&
            bindingDesc.resourceType != ShaderResourceType::Texture)
        {
            LOG_ERROR("Invalid shader resource type at binding %i", i);
            return false;
        }

        mBindings.push_back(desc.resourceBindings[i]);
    }

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
    mSamplers.resize(mBindingSet->mBindings.size());

    return true;
}

bool ResourceBindingInstance::WriteTextureView(size_t slot, ITexture* texture, ISampler* sampler)
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

    Sampler* samp = dynamic_cast<Sampler*>(sampler);
    if (!samp || !samp->mSamplerState)
    {
        LOG_ERROR("Invalid sampler state");
        return false;
    }

    mViews[slot] = tex->mSRV.get();
    mSamplers[slot] = samp->mSamplerState.get();
    return true;
}

bool ResourceBindingInstance::WriteCBufferView(size_t slot, IBuffer* buffer)
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

} // namespace Renderer
} // namespace NFE
