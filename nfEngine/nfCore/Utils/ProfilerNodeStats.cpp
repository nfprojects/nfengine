/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  ProfilerNodeStats definitions
 */

#include "PCH.hpp"

#include "ProfilerNodeStats.hpp"

namespace NFE {
namespace Util {

ProfilerNodeStats::ProfilerNodeStats()
    : time(0.0)
    , visitCount(0)
{
}

ProfilerNodeStats& ProfilerNodeStats::operator+=(const ProfilerNodeStats& rh)
{
    this->time += rh.time;
    this->visitCount += rh.visitCount;
    return *this;
}

ProfilerNodeStats& ProfilerNodeStats::operator-=(const ProfilerNodeStats& rh)
{
    this->time -= rh.time;
    this->visitCount -= rh.visitCount;
    return *this;
}

ProfilerNodeStats ProfilerNodeStats::operator+(const ProfilerNodeStats& rh) const
{
    ProfilerNodeStats result;
    result.time = this->time + rh.time;
    result.visitCount = this->visitCount + rh.visitCount;
    return result;
}

ProfilerNodeStats ProfilerNodeStats::operator-(const ProfilerNodeStats& rh) const
{
    ProfilerNodeStats result;
    result.time = this->time - rh.time;
    result.visitCount = this->visitCount - rh.visitCount;
    return result;
}

void ProfilerNodeStats::Reset()
{
    time = 0.0;
    visitCount = 0;
}

} // namespace Util
} // namespace NFE
