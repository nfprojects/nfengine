#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {

/**
 * 4-element boolean vector
 */
struct NFE_ALIGN(16) VectorBool4
{
    VectorBool4() = default;

    NFE_FORCE_INLINE explicit VectorBool4(bool x, bool y, bool z, bool w);
    NFE_FORCE_INLINE explicit VectorBool4(int x, int y, int z, int w);

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_FORCE_INLINE const VectorBool4 Swizzle() const;

    // combine into 4-bit mask
    NFE_FORCE_INLINE int32 GetMask() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE bool All3() const;
    NFE_FORCE_INLINE bool None3() const;
    NFE_FORCE_INLINE bool Any3() const;

    NFE_FORCE_INLINE const VectorBool4 operator & (const VectorBool4 rhs) const;
    NFE_FORCE_INLINE const VectorBool4 operator | (const VectorBool4 rhs) const;
    NFE_FORCE_INLINE const VectorBool4 operator ^ (const VectorBool4 rhs) const;

    NFE_FORCE_INLINE bool operator == (const VectorBool4 rhs) const;

private:
    friend struct Vector4;
    friend struct VectorInt4;
    friend struct VectorBool8;

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE VectorBool4(const __m128 other) : v(other) { }
    NFE_FORCE_INLINE VectorBool4(const __m128i other) : v(_mm_castsi128_ps(other)) { }
    NFE_FORCE_INLINE operator __m128() const { return v; }
    NFE_FORCE_INLINE operator __m128i() const { return _mm_castps_si128(v); }

    __m128 v;
#else
    bool b[4];
#endif // NFE_USE_SSE
};

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_SSE
#include "VectorBool4ImplSSE.hpp"
#else
#include "VectorBool4ImplNaive.hpp"
#endif
