#include "PCH.hpp"
#include "RenderGraphResourceAllocator.hpp"
#include "Renderer.hpp"
#include "../../Common/Containers/StringView.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

RenderGraphResourceRef::RenderGraphResourceRef(RenderGraphResourceName name)
    : mName(name)
{}

RenderGraphResourceRef::RenderGraphResourceRef(RenderGraphResourceName name, const TexturePtr& texture)
    : mName(name)
    , mTexture(texture)
{}

RenderGraphResourceRef::RenderGraphResourceRef(RenderGraphResourceName name, const BufferPtr& buffer)
    : mName(name)
    , mBuffer(buffer)
{}

RenderGraphScopedResourceRef::RenderGraphScopedResourceRef(const RenderGraphResourceRef& resourceRef, RenderGraphResourceAllocator& allocator)
    : mResourceRef(resourceRef)
    , mAllocator(allocator)
{
    mAllocator.EndUseResource(mResourceRef.GetName());
}

RenderGraphScopedResourceRef::~RenderGraphScopedResourceRef()
{
    mAllocator.EndUseResource(mResourceRef.GetName());
}



RenderGraphResourceAllocator::RenderGraphResourceAllocator()
    : mPreparePhase(true)
{
}

RenderGraphResourceAllocator::~RenderGraphResourceAllocator()
{
}

RenderGraphResourceRef RenderGraphResourceAllocator::Declare(RenderGraphResourceName name, const TextureDesc& desc)
{
    const uint64 nameHash = GetHash(StringView(name));

    if (mPreparePhase)
    {
        NFE_ASSERT(!mResources.Exists(nameHash), "Texture '%s' was already declared", name);

        ResourceData resourceData;
        resourceData.type = ResourceType::Texture;
        resourceData.name = name;
        resourceData.textureDesc = desc;
        mResources.Insert(nameHash, std::move(resourceData));

        return RenderGraphResourceRef(name);
    }
    else
    {
        const auto iter = mResources.Find(nameHash);

        NFE_ASSERT(iter != mResources.End(), "Texture '%s' was not declared in prepare phase", name);
        NFE_ASSERT(iter->second.type == ResourceType::Texture, "Resource '%s' was not declared as a texture", name);
        NFE_ASSERT(iter->second.textureDesc == desc, "Texture '%s' desc does not match", name);

        return RenderGraphResourceRef(name, iter->second.texture);
    }
}

RenderGraphResourceRef RenderGraphResourceAllocator::Declare(RenderGraphResourceName name, const BufferDesc& desc)
{
    const uint64 nameHash = GetHash(StringView(name));

    if (mPreparePhase)
    {
        NFE_ASSERT(!mResources.Exists(nameHash), "Buffer '%s' was already declared", name);

        ResourceData resourceData;
        resourceData.type = ResourceType::Buffer;
        resourceData.name = name;
        resourceData.bufferDesc = desc;
        mResources.Insert(nameHash, std::move(resourceData));

        return RenderGraphResourceRef(name);
    }
    else
    {
        const auto iter = mResources.Find(nameHash);

        NFE_ASSERT(iter != mResources.End(), "Buffer '%s' was not declared in prepare phase", name);
        NFE_ASSERT(iter->second.type == ResourceType::Buffer, "Resource '%s' was not declared as a buffer", name);
        NFE_ASSERT(iter->second.bufferDesc == desc, "Buffer '%s' desc does not match", name);

        return RenderGraphResourceRef(name, iter->second.buffer);
    }
}

RenderGraphResourceRef RenderGraphResourceAllocator::GetResource(RenderGraphResourceName name)
{
    const uint64 nameHash = GetHash(StringView(name));
    const auto iter = mResources.Find(nameHash);
    NFE_ASSERT(iter != mResources.End(), "Resource '%s' was not declared", name);

    if (iter->second.type == ResourceType::Buffer)
    {
        return RenderGraphResourceRef(name, iter->second.buffer);
    }
    else
    {
        return RenderGraphResourceRef(name, iter->second.texture);
    }
}

RenderGraphScopedResourceRef RenderGraphResourceAllocator::GetScopedResource(ResourceName name)
{
    return RenderGraphScopedResourceRef(GetResource(name), *this);
}

void RenderGraphResourceAllocator::BeginUseResource(RenderGraphResourceName name)
{
    // TODO event counter must be tracked per command list
    uint32 eventID = mEventCounter++;

    const uint64 nameHash = GetHash(StringView(name));
    const auto iter = mResources.Find(nameHash);
    NFE_ASSERT(iter != mResources.End(), "Resource '%s' was not declared", name);

    iter->second.beginUseMarker = Math::Min(iter->second.beginUseMarker, eventID);
}

void RenderGraphResourceAllocator::EndUseResource(RenderGraphResourceName name)
{
    // TODO event counter must be tracked per command list
    uint32 eventID = mEventCounter++;

    const uint64 nameHash = GetHash(StringView(name));
    const auto iter = mResources.Find(nameHash);
    NFE_ASSERT(iter != mResources.End(), "Resource '%s' was not declared", name);

    iter->second.endUseMarker = Math::Max(iter->second.endUseMarker, eventID);

    NFE_ASSERT(iter->second.beginUseMarker < iter->second.endUseMarker, "BeginUseResource / EndUseResource mismatch");
}

void RenderGraphResourceAllocator::Reset()
{
    // move resources to the cache
    for (auto& iter : mResources)
    {
        mResourcesCache.EmplaceBack(std::move(iter.second));
    }

    mResources.Clear();

    mPreparePhase = true;
    mEventCounter = 0;
}

void RenderGraphResourceAllocator::Resolve()
{
    NFE_ASSERT(mPreparePhase, "RenderGraphResourceAllocator is not in prepare phase");

    mPreparePhase = false;

    // at this point we have all resources declarations and their lifetimes

    // TODO this is super trivial and naive !!!!
    // TODO reuse same object for non-overlapping resources
    // TODO memory aliasing, optimal allocation to minimize memory usage
    for (auto& iter : mResources)
    {
        if (iter.second.beginUseMarker == UINT32_MAX && iter.second.endUseMarker == 0)
        {
            NFE_LOG_WARNING("Render graph resource '%s' was declared but never used", iter.second.name);
            continue;
        }

        NFE_ASSERT(iter.second.endUseMarker < mEventCounter, "Resource '%s' has missing/corrupted end use marker", iter.second.name);
        NFE_ASSERT(iter.second.beginUseMarker < iter.second.endUseMarker, "Resource '%s' has missing/corrupted begin use marker", iter.second.name);

        bool foundInCache = false;

        // first, find same resource in the cache
        for (ResourceData& cachedResource : mResourcesCache)
        {
            if (iter.second.type == cachedResource.type)
            {
                if (iter.second.type == ResourceType::Buffer)
                {
                    if (iter.second.bufferDesc == cachedResource.bufferDesc)
                    {
                        iter.second.buffer = std::move(cachedResource.buffer);
                        cachedResource.bufferDesc = {};
                        foundInCache = true;
                        break;
                    }
                }
                else if(iter.second.type == ResourceType::Texture)
                {
                    if (iter.second.textureDesc == cachedResource.textureDesc)
                    {
                        iter.second.texture = std::move(cachedResource.texture);
                        cachedResource.textureDesc = {};
                        foundInCache = true;
                        break;
                    }
                }
                else
                {
                    NFE_FATAL("Invalid resource type");
                }
            }
        }

        // not found in cache - create new resource
        if (!foundInCache)
        {
            if (iter.second.type == ResourceType::Buffer)
            {
                iter.second.buffer = Renderer::GetInstance().GetDevice()->CreateBuffer(iter.second.bufferDesc);
            }
            else if (iter.second.type == ResourceType::Texture)
            {
                iter.second.texture = Renderer::GetInstance().GetDevice()->CreateTexture(iter.second.textureDesc);
            }
            else
            {
                NFE_FATAL("Invalid resource type");
            }
        }
    }

    mResourcesCache.Clear();
}


} // namespace Scene
} // namespace NFE
