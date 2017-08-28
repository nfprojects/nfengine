/**
* @file
* @author Witek902 (witek902@gmail.com)
* @brief  Definition of hashing functions
*/

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

/**
* Default hashing function - cast from 'in' type to 'out' type.
*/
template<typename InType, typename OutType>
struct Hash
{
    static_assert(std::is_integral<OutType>::value, "Hash function result must be of integral type");

    OutType operator() (const InType& in) const
    {
        return static_cast<OutType>(in);
    }
};

// TODO hash functions for:
//  floating point
//  String
//  pointers
//  DynArray


} // namespace Common
} // namespace NFE
