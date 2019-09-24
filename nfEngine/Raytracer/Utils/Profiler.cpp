#include "PCH.h"
#include "Profiler.h"
#include "../nfCommon/System/Timer.hpp"
#include "../nfCommon/Utils/ScopedLock.hpp"
#include "../nfCommon/Logger/Logger.hpp"
#include "../nfCommon/Containers/HashMap.hpp"

namespace NFE {
namespace RT {

using namespace Common;

Profiler& Profiler::GetInstance()
{
    static Profiler profiler;
    return profiler;
}

void Profiler::RegisterEntry(ScopedEntry& entry)
{
    NFE_SCOPED_LOCK(mLock);
    entry.nextEntry = mFirstEntry;
    mFirstEntry = &entry;
}

void Profiler::Collect(DynArray<ProfilerResult>& outResult)
{
    HashMap<const char*, ScopedEntryData> entriesMap;
    {
        NFE_SCOPED_LOCK(mLock);
        for (ScopedEntry* entry = mFirstEntry; entry != nullptr; entry = entry->nextEntry)
        {
            entriesMap.Insert(entry->name, ScopedEntryData());
            entriesMap[entry->name] += entry->data;
        }
    }

    for (const auto& iter : entriesMap)
    {
        ProfilerResult result;
        result.scopeName = iter.first;
        result.avgTime = iter.second.accumulatedTicks * Timer::gPeriod / iter.second.count;
        result.minTime = iter.second.minTick * Timer::gPeriod;
        result.count = iter.second.count;
        outResult.PushBack(result);
        //NFE_LOG_INFO("Profiler: %s count=%llu, avg=%.3f us, min=%.3fus, ", iter.first, data.count, avgTime, minTime);
    }
}

void Profiler::ResetAll()
{
    NFE_SCOPED_LOCK(mLock);
    for (ScopedEntry* entry = mFirstEntry; entry != nullptr; entry = entry->nextEntry)
    {
        entry->data = ScopedEntryData();
    }
}

ScopedEntry::ScopedEntry(const char* name)
    : name(name)
{
    Profiler::GetInstance().RegisterEntry(*this);
}


} // namespace RT
} // namespace NFE
