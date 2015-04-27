/**
 * @file   SystemInfo.hpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API declarations.
 */

#pragma once

#include "nfCommon.hpp"
#include "Timer.hpp"

#if defined(__LINUX__) | defined(__linux__)
#include "sys/sysinfo.h"
#include <unistd.h>
#endif // defined(__LINUX__) | defined(__linux__)


namespace NFE {
namespace Common {

struct CpuidFeature
{
    int AddressNo;
    int FeatureNo;
    CpuidFeature() : AddressNo(0), FeatureNo(0)
    {};
    CpuidFeature(int x, int y) : AddressNo(x), FeatureNo(y)
    {}
};

/**
 * Class used to gather information about hardware
 */
class NFCOMMON_API SystemInfo
{
private:
    static std::unique_ptr<SystemInfo> mInstance;
    SystemInfo();

    std::map<std::string, CpuidFeature> mCpuidFeatureMap;
    std::string mCPUBrand;
    uint64_t mCPUCoreNo;
    uint64_t mPageSize;
    uint64_t mCacheLineSize;
    uint64_t mCPUSpeedMHz;
    uint64_t mMemTotalPhysKb;
    uint64_t mMemFreePhysKb;
    uint64_t mMemTotalVirtKb;
    uint64_t mMemFreeVirtKb;
    uint64_t mCpuidFeatures[5];

    void Cpuid(int cpuInfo[4], int function_id);
    uint64_t Rdtsc();

    void InitCPUInfoCommon();
    void InitMap();
    void InitCPUInfoPlatform();
    void InitMemoryInfo();

    bool CheckFeature(CpuidFeature feature) const;

public:
    static SystemInfo* Instance();

    /**
     * Update mMemFree fields
     * @return Total free memory in kb (physical + virtual)
     */
    uint64_t GetFreeMemoryKb();

    /**
     * Check if feature is supported
     * @return True if feature is supported, False otherwise
     */
    bool IsFeatureSupported(const std::string& featureName) const;

    /**
     * Get class fields
     */
    const std::string& GetCPUBrand() const;
    uint64_t GetCPUCoreNo() const;
    uint64_t GetPageSize() const;
    uint64_t GetCacheLineSize() const;
    uint64_t GetCPUSpeedMHz() const;
    uint64_t GetMemTotalPhysKb() const;
    uint64_t GetMemTotalVirtKb() const;
    uint64_t GetMemFreePhysKb() const;
    uint64_t GetMemFreeVirtKb() const;

    std::string ConstructAllInfoString();
};

} // namespace Common
} // namespace NFE