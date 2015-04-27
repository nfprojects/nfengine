/**
 * @file   SystemInfo.hpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API declarations.
 */

#pragma once

#include "nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * Class used to gather information about hardware
 */
class NFCOMMON_API SystemInfo
{
private:
    std::string mCPUBrand;
    uint64_t mCPUCoreNo;
    uint64_t mPageSize;
    uint64_t mCacheLineSize;
    uint64_t mCPUSpeedMHz;
    uint64_t mMemTotalPhysKb;
    uint64_t mMemFreePhysKb;
    uint64_t mMemTotalVirtKb;
    uint64_t mMemFreeVirtKb;
public:
    SystemInfo();

    /**
     * Update mMemFree fields
     * @return Total free memory in kb (physical + virtual)
     */
    uint64_t GetFreeMemoryKb();

    /**
     * Get class fields
     */
    std::string GetCPUBrand();
    uint64_t GetCPUCoreNo();
    uint64_t GetPageSize();
    uint64_t GetCacheLineSize();
    uint64_t GetCPUSpeedMHz();
    uint64_t GetMemTotalPhysKb();
    uint64_t GetMemTotalVirtKb();
    uint64_t GetMemFreePhysKb();
    uint64_t GetMemFreeVirtKb();
};

} // namespace Common
} // namespace NFE