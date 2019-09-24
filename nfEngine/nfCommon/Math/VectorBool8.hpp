#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {

/**
 * 8-element boolean vector
 */
struct NFE_ALIGN(32) VectorBool8
{
    VectorBool8() = default;

    NFE_FORCE_INLINE VectorBool8(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7);
    NFE_FORCE_INLINE VectorBool8(const VectorBool4& low, const VectorBool4& high);

#ifdef NFE_USE_AVX
    NFE_FORCE_INLINE VectorBool8(const __m256 other) : v(other) { }
    NFE_FORCE_INLINE VectorBool8(const __m256i other) : v(_mm256_castsi256_ps(other)) { }
    NFE_FORCE_INLINE operator __m256() const { return v; }
    NFE_FORCE_INLINE operator __m256i() const { return _mm256_castps_si256(v); }
#endif // NFE_USE_SSE

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    // combine into 8-bit mask
    NFE_FORCE_INLINE int GetMask() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE const VectorBool8 operator & (const VectorBool8 rhs) const;
    NFE_FORCE_INLINE const VectorBool8 operator | (const VectorBool8 rhs) const;
    NFE_FORCE_INLINE const VectorBool8 operator ^ (const VectorBool8 rhs) const;

    NFE_FORCE_INLINE bool operator == (const VectorBool8 rhs) const;

private:
    friend struct Vector8;
    friend struct VectorInt8;

#ifdef NFE_USE_AVX
    __m256 v;
#else
    bool b[8];
#endif // NFE_USE_SSE
};

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_AVX
#include "VectorBool8ImplAVX.hpp"
#else
#include "VectorBool8ImplNaive.hpp"
#endif
