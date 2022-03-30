#include "PCH.hpp"
#include "DescriptorSetCache.hpp"

#include "API/Device.hpp"
#include "Internal/Debugger.hpp"
#include "Internal/Translations.hpp"

#include "Engine/Common/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

DescriptorSetCacheEntry::~DescriptorSetCacheEntry()
{
    if (sets.Size() != 0)
    {
        vkFreeDescriptorSets(gDevice->GetDevice(), pool, sets.Size(), sets.Data());
    }
}



DescriptorSetCache::DescriptorSetCache()
    : mDescriptorPool(VK_NULL_HANDLE)
    , mCacheIDCounter(0)
    , mCacheLock()
    , mCache()
{
}

DescriptorSetCache::~DescriptorSetCache()
{
    Release();
}

DescriptorSetCacheEntry& DescriptorSetCache::GetNextCacheEntry(DescriptorSetCollectionID& id)
{
    NFE_SCOPED_LOCK(mCacheLock);

    id = ++mCacheIDCounter;
    auto result = mCache.Insert(id, DescriptorSetCacheEntry(mDescriptorPool));

    return result.iterator->second;
}

bool DescriptorSetCache::Init()
{
    VkResult result = VK_SUCCESS;

    // TODO resize these if we run out of space
    Common::StaticArray<VkDescriptorPoolSize, 4> descPoolSizes;
    descPoolSizes.Resize(4);
    descPoolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    descPoolSizes[0].descriptorCount = 32;
    descPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descPoolSizes[1].descriptorCount = 256;
    descPoolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descPoolSizes[2].descriptorCount = 256;
    descPoolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descPoolSizes[3].descriptorCount = 8;

    VkDescriptorPoolCreateInfo descPoolInfo;
    VK_ZERO_MEMORY(descPoolInfo);
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.poolSizeCount = descPoolSizes.Size();
    descPoolInfo.pPoolSizes = descPoolSizes.Data();
    descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    for (const auto& s: descPoolSizes)
        descPoolInfo.maxSets += s.descriptorCount;
    result = vkCreateDescriptorPool(gDevice->GetDevice(), &descPoolInfo, nullptr, &mDescriptorPool);
    CHECK_VKRESULT(result, "Failed to create Descriptor Pool");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mDescriptorPool), VK_OBJECT_TYPE_DESCRIPTOR_POOL, "DescriptorSetCache-DescriptorPool");

    return true;
}

void DescriptorSetCache::Release()
{
    mCache.Clear();

    if (mDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(gDevice->GetDevice(), mDescriptorPool, nullptr);
        mDescriptorPool = VK_NULL_HANDLE;
    }
}

DescriptorSetCollectionID DescriptorSetCache::AllocateDescriptorSets(const DescriptorSetLayoutCollection& layouts)
{
    // TODO this whole thing is now done in a naive way, which doesn't actually "cache"
    //      sets. Make it actually cache and reuse Sets to avoid excessive allocations.
    DescriptorSetCollectionID ret;
    DescriptorSetCacheEntry& cacheEntry = GetNextCacheEntry(ret);

    cacheEntry.sets.Resize(layouts.Size());

    VkDescriptorSetAllocateInfo allocInfo;
    VK_ZERO_MEMORY(allocInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pSetLayouts = layouts.Data();
    allocInfo.descriptorSetCount = layouts.Size();
    allocInfo.descriptorPool = mDescriptorPool;
    // TODO use temporary pool? might be easier to manage in case of fragmentation
    VkResult result = vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, cacheEntry.sets.Data());
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Failed to allocate temporary descriptor sets: %d (%s)", result, TranslateVkResultToString(result));
        return UINT32_MAX;
    }

    return ret;
}

DescriptorSetCollection& DescriptorSetCache::GetDescriptorSets(DescriptorSetCollectionID id)
{
    NFE_ASSERT(mCache.Exists(id), "Descriptor Set Collection not found");
    return mCache[id].sets;
}

void DescriptorSetCache::FreeDescriptorSets(DescriptorSetCollectionID id)
{
    NFE_SCOPED_LOCK(mCacheLock);

    bool freed = mCache.Erase(id);
    NFE_ASSERT(freed, "Attempted to double-free a Descriptor Set Collection, ID = %d", id);
}

} // namespace Renderer
} // namespace NFE