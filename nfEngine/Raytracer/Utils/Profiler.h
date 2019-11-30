#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Containers/DynArray.hpp"
#include "../../nfCommon/Math/Math.hpp"
#include "../../nfCommon/System/Mutex.hpp"
#include "../../../nfEngineDeps/tracy/Tracy.hpp"

namespace NFE {
namespace RT {

class ScopedEntry;

struct ProfilerResult
{
    const char* scopeName = nullptr;
    double avgTime = 0.0;
    double minTime = 0.0;
    uint64 count = 0;
};

class NFE_RAYTRACER_API Profiler
{
public:
    static Profiler& GetInstance();
    void RegisterEntry(ScopedEntry& entry);
    void Collect(Common::DynArray<ProfilerResult>& outResult);
    void ResetAll();

private:
    Profiler() = default;

    Common::Mutex mLock;
    ScopedEntry* mFirstEntry;
};

struct ScopedEntryData
{
    uint64 minTick = UINT64_MAX;
    uint64 accumulatedTicks = 0;
    uint64 count = 0;

    ScopedEntryData& operator += (const ScopedEntryData& other)
    {
        minTick = Math::Min(minTick, other.accumulatedTicks);
        accumulatedTicks += other.accumulatedTicks;
        count += other.count;
        return *this;
    }
};

class NFE_RAYTRACER_API ScopedEntry
{
public:
    NFE_FORCE_NOINLINE ScopedEntry(const char* name);

    const char* name;
    ScopedEntry* nextEntry = nullptr;
    ScopedEntryData data;
};


class NFE_RAYTRACER_API ScopedTimer
{
public:
    NFE_FORCE_INLINE ScopedTimer(ScopedEntry& entry)
        : mEntry(entry)
    {
        QueryPerformanceCounter(&mStart);
    }

    NFE_FORCE_INLINE ~ScopedTimer()
    {
        LARGE_INTEGER stop;
        QueryPerformanceCounter(&stop);
        const uint64 ticks = stop.QuadPart - mStart.QuadPart;

        if (ticks > 0)
        {
            mEntry.data.minTick = Math::Min(mEntry.data.minTick, ticks);
        }

        mEntry.data.accumulatedTicks += ticks;
        mEntry.data.count++;
    }

    void ReportAndReset();

private:
    LARGE_INTEGER mStart;
    ScopedEntry& mEntry;
};

} // namespace RT
} // namespace NFE


#define NFE_SCOPED_TIMER(name) \
    ZoneScopedN(#name); \
    thread_local ScopedEntry entry##name(#name); \
    ScopedTimer scopedTimer(entry##name);
