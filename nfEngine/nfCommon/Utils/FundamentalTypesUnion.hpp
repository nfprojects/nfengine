#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

using FundamentalTypesUnion = union
{
    bool b;
    int8 i8;
    int16 i16;
    int32 i32;
    int64 i64;
    uint8 u8;
    uint16 u16;
    uint32 u32;
    uint64 u64;
    float f;
    double d;
};

template<typename T>
T ExtractValue(const FundamentalTypesUnion& u);

template<> NFE_FORCE_INLINE int8 ExtractValue(const FundamentalTypesUnion& u) { return u.i8; }
template<> NFE_FORCE_INLINE int16 ExtractValue(const FundamentalTypesUnion& u) { return u.i16; }
template<> NFE_FORCE_INLINE int32 ExtractValue(const FundamentalTypesUnion& u) { return u.i32; }
template<> NFE_FORCE_INLINE int64 ExtractValue(const FundamentalTypesUnion& u) { return u.i64; }
template<> NFE_FORCE_INLINE uint8 ExtractValue(const FundamentalTypesUnion& u) { return u.u8; }
template<> NFE_FORCE_INLINE uint16 ExtractValue(const FundamentalTypesUnion& u) { return u.u16; }
template<> NFE_FORCE_INLINE uint32 ExtractValue(const FundamentalTypesUnion& u) { return u.u32; }
template<> NFE_FORCE_INLINE uint64 ExtractValue(const FundamentalTypesUnion& u) { return u.u64; }
template<> NFE_FORCE_INLINE float ExtractValue(const FundamentalTypesUnion& u) { return u.f; }
template<> NFE_FORCE_INLINE double ExtractValue(const FundamentalTypesUnion& u) { return u.d; }

} // namespace Common
} // namespace NFE
