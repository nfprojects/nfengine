/**
* @file
* @author Witek902 (witek902@gmail.com)
* @brief  Definition of hashing functions
*/

#pragma once

#include "../nfCommon.hpp"
#include "../Math/Math.hpp"


namespace NFE {
namespace Common {

/**
 * Generic hash function.
 */
template<typename T>
uint32 GetHash(const T& x);


template<>
NFE_INLINE constexpr uint32 GetHash<uint8>(const uint8& x)
{
    return static_cast<uint32>(x);
}

template<>
NFE_INLINE constexpr uint32 GetHash<uint16>(const uint16& x)
{
    return static_cast<uint32>(x);
}

template<>
NFE_INLINE constexpr uint32 GetHash<uint32>(const uint32& x)
{
    return x;
}

template<>
NFE_INLINE constexpr uint32 GetHash<uint64>(const uint64& x)
{
    return static_cast<uint32>(x) ^ static_cast<uint32>(x >> 32);
}

template<>
NFE_INLINE constexpr uint32 GetHash<int8>(const int8& x)
{
    return static_cast<uint32>(x);
}

template<>
NFE_INLINE constexpr uint32 GetHash<int16>(const int16& x)
{
    return static_cast<uint32>(x);
}

template<>
NFE_INLINE constexpr uint32 GetHash<int32>(const int32& x)
{
    return static_cast<uint32>(x);
}

template<>
NFE_INLINE constexpr uint32 GetHash<int64>(const int64& x)
{
    return GetHash(static_cast<uint64>(x));
}

/**
 * Hash function for pointers.
 */
template<typename T>
uint32 constexpr GetHash(const T* x)
{
    return GetHash(reinterpret_cast<size_t>(x));
}

template<typename T>
uint32 constexpr GetHash(T* x)
{
    return GetHash(reinterpret_cast<size_t>(x));
}

template<>
NFE_INLINE uint32 GetHash<float>(const float& x)
{
    Math::Bits32 bits;
    bits.f = x;
    return bits.ui;
}

template<>
NFE_INLINE uint32 GetHash<double>(const double& x)
{
    Math::Bits64 bits;
    bits.f = x;
    return GetHash(bits.ui);
}


// TODO hash functions for:
//  String
//  pointers
//  DynArray


} // namespace Common
} // namespace NFE
