/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Comparator template class used for comparing elements in containers, sorting, etc.
 */

#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Common {


// default values comparator
template<typename T>
struct DefaultComparator
{
    NFE_INLINE bool Less(const T& left, const T& right) const
    {
        return left < right;
    }

    NFE_INLINE bool Equal(const T& left, const T& right) const
    {
        return left == right;
    }
};

} // namespace Common
} // namespace NFE
