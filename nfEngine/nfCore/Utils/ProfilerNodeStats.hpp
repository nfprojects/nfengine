/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  ProfilerNodeStats declarations
 */

#pragma once

namespace NFE {
namespace Util {

/**
 * Structure with statistics gathered by every node.
 */
struct ProfilerNodeStats
{
    double time;                ///< Time between StartScope and StopScope calls
    unsigned int visitCount;    ///< Amount of times the Node has been visited

    ProfilerNodeStats();

    ProfilerNodeStats& operator+=(const ProfilerNodeStats& rh);
    ProfilerNodeStats& operator-=(const ProfilerNodeStats& rh);

    ProfilerNodeStats operator+(const ProfilerNodeStats& rh) const;
    ProfilerNodeStats operator-(const ProfilerNodeStats& rh) const;

    void Reset();
};

} // namespace Util
} // namespace NFE
