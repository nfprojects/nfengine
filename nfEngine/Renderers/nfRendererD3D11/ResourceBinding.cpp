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

bool ResourceBindingSet::IsBindingOverlapping(const ResourceBindingDesc& bindingDesc) const
{
    for (const auto& desc : mBindings)
        if (desc.slot == bindingDesc.slot)
            return true;
    return false;
}

bool ResourceBindingSet::IsBindingSetOverlapping(const ResourceBindingSet* set) const
{
    for (const auto& desc : set->mBindings)
        if (IsBindingOverlapping(desc))
            if (mShaderVisibility == ShaderType::All ||
                set->mShaderVisibility == ShaderType::All ||
                mShaderVisibility == set->mShaderVisibility)
                return true;
    return false;
}

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

        for (size_t j = 0; j < desc.numBindings; ++j)
        {
            if (i == j)
                continue;

            if (bindingDesc.slot == desc.resourceBindings[j].slot &&
                bindingDesc.resourceType == desc.resourceBindings[j].resourceType)
            {
                LOG_ERROR("Binding set has two same slots: %i and %j", i, j);
                return false;
            }
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

        for (size_t j = 0; j < i; ++j)
        {
            if (mBindingSets[j] == bindingSet)
            {
                LOG_ERROR("Same binding sets (%zu and %zu) can't be reused in a binding layout",
                          j, i);
                return false;
            }

            if (bindingSet->IsBindingSetOverlapping(mBindingSets[j]))
            {
                LOG_ERROR("Resource binding slots are overlapping (sets %zu and %zu)", j, i);
                return false;
            }
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

bool ResourceBindingInstance::WriteTextureView(size_t slot, ITexture* texture)
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
