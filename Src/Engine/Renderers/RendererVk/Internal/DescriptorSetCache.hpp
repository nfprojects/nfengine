#pragma once

#include "Defines.hpp"

#include "Engine/Common/Containers/StaticArray.hpp"
#include "Engine/Common/Containers/HashMap.hpp"
#include "Engine/Common/System/RWLock.hpp"


namespace NFE {
namespace Renderer {

struct DescriptorSetCacheEntry
{
    VkDescriptorPool pool;
    DescriptorSetCollection sets;

    // TODO HashMap requires this constructor for using HashMap::Exists
    //      and others, but I'd like to not have it here. See if this can be fixed.
    DescriptorSetCacheEntry()
        : pool(VK_NULL_HANDLE)
        , sets()
    {}

    DescriptorSetCacheEntry(VkDescriptorPool pool)
        : pool(pool)
        , sets()
    {}

    ~DescriptorSetCacheEntry();
};

class DescriptorSetCache
{
private:
    VkDescriptorPool mDescriptorPool;
    DescriptorSetCollectionID mCacheIDCounter;
    Common::RWLock mCacheLock;
    Common::HashMap<DescriptorSetCollectionID, DescriptorSetCacheEntry> mCache;

    DescriptorSetCacheEntry& GetNextCacheEntry(DescriptorSetCollectionID& id);

public:
    DescriptorSetCache();
    ~DescriptorSetCache();

    bool Init();
    void Release();

    DescriptorSetCollectionID AllocateDescriptorSets(const DescriptorSetLayoutCollection& layouts);
    DescriptorSetCollection& GetDescriptorSets(DescriptorSetCollectionID id);
    void FreeDescriptorSets(DescriptorSetCollectionID id);
};

} // namespace Renderer
} // namespace NFE