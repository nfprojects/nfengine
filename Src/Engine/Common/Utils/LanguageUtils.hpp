#pragma once

#include "../nfCommon.hpp"

#include <typeinfo>
#include <cstring>

namespace NFE {

// Check if a type is defined
template<typename, typename = void>
constexpr bool IsTypeDefined = false;
template<typename T>
constexpr bool IsTypeDefined<T, std::void_t<decltype(sizeof(T))>> = true;


// Return size of an array
template<class T, size_t N>
constexpr size_t ArraySize(const T(&)[N])
{
    return N;
}


// Reinterpret cast in a safe, defined way.
// TODO this will get implemented in C++20
template <class To, class From>
NFE_FORCE_INLINE
typename std::enable_if<
    (sizeof(To) == sizeof(From)) &&
    std::is_trivially_copyable_v<From> &&
    std::is_trivial_v<To> &&
    (std::is_copy_constructible_v<To> || std::is_move_constructible_v<To>),
To>::type BitCast(const From& src) noexcept
{
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

#define DEFINE_ENUM_OPERATORS(EnumType) \
    NFE_INLINE EnumType operator & (EnumType a, EnumType b) { return static_cast<EnumType>(static_cast<uint32>(a) & static_cast<uint32>(b)); } \
    NFE_INLINE EnumType operator | (EnumType a, EnumType b) { return static_cast<EnumType>(static_cast<uint32>(a) | static_cast<uint32>(b)); } \
    NFE_INLINE EnumType operator ^ (EnumType a, EnumType b) { return static_cast<EnumType>(static_cast<uint32>(a) ^ static_cast<uint32>(b)); } \
    NFE_INLINE EnumType& operator |= (EnumType& lhs, EnumType rhs) { lhs = static_cast<EnumType>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs)); return lhs; } \
    NFE_INLINE EnumType& operator &= (EnumType& lhs, EnumType rhs) { lhs = static_cast<EnumType>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs)); return lhs; } \
    NFE_INLINE EnumType& operator ^= (EnumType& lhs, EnumType rhs) { lhs = static_cast<EnumType>(static_cast<uint32>(lhs) ^ static_cast<uint32>(rhs)); return lhs; } \

} // namespace NFE
