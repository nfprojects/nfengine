#pragma once

#include "../Raytracer.h"
#include "../../Common/Containers/DynArray.hpp"
#include "../../Common/Math/Math.hpp"
#include "../../Common/System/Mutex.hpp"

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
    double minTick = std::numeric_limits<double>::max();
    double accumulatedTime = 0.0;
    uint64 count = 0;

    ScopedEntryData& operator += (const ScopedEntryData& other)
    {
        minTick = Math::Min(minTick, other.accumulatedTime);
        accumulatedTime += other.accumulatedTime;
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

// TODO
/*
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
*/

} // namespace RT
} // namespace NFE


// TODO
#define NFE_SCOPED_TIMER(name)
// #define NFE_SCOPED_TIMER(name) \
//     thread_local ScopedEntry entry##name(#name); \
//     ScopedTimer scopedTimer(entry##name);
