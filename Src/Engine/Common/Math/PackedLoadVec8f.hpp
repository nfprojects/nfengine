#pragma once

#include "Packed.hpp"
#include "Vec8f.hpp"
#include "Vec8i.hpp"

namespace NFE {
namespace Math {

// load 8 halfs into Vec8f
NFE_FORCE_INLINE const Vec8f LoadVec8f(const Half4* src)
{
#ifdef NFE_USE_FP16C
    return _mm256_cvtph_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(src)));
#else // NFE_USE_FP16C
    return Vec8f(
        src[0],x.ToFloat(), src[0].y.ToFloat(), src[0].z.ToFloat(), src[0].w.ToFloat(),
        src[1].x.ToFloat(), src[1].y.ToFloat(), src[1].z.ToFloat(), src[1].w.ToFloat()
    );
#endif // NFE_USE_FP16C
}

static const Vec8f LoadVec8f(const PackedUFloat3_9_9_9_5* input)
{
    const Vec8ui vInput(Vec4ui(input[0].v), Vec4ui(input[1].v));

    const Vec8ui mantissaShift(0, 9, 18, 0, 0, 9, 18, 0);
    const Vec8ui mantissaMask(0x1FF);
    const Vec8i mantissa((vInput >> mantissaShift) & mantissaMask);

    const Vec8i exponent(vInput >> 27);
    const Vec8i base = Vec8i(0x33800000) + (exponent << 23);

    return base.AsVec8f() * mantissa.ConvertToVec8f();
}

} // namespace Math
} // namespace NFE
