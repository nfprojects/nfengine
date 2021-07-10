#pragma once

#include "Packed.hpp"
#include "Vec4f.hpp"
#include "Vec4i.hpp"
#include "Half.hpp"

namespace NFE {
namespace Math {


NFE_FORCE_INLINE const Vec4f Vec4f_Load_Vec2f_Unsafe(const Vec2f* src)
{
    return Vec4f(reinterpret_cast<const float*>(src));
}

NFE_FORCE_INLINE const Vec4f Vec4f_Load_Vec3f_Unsafe(const Vec3f* src)
{
    return Vec4f(reinterpret_cast<const float*>(src));
}

NFE_FORCE_INLINE const Vec4f Vec4f_Load_Vec2f_Unsafe(const Vec2f& src)
{
    return Vec4f(reinterpret_cast<const float*>(&src));
}

NFE_FORCE_INLINE const Vec4f Vec4f_Load_Vec3f_Unsafe(const Vec3f& src)
{
    return Vec4f(reinterpret_cast<const float*>(&src));
}

NFE_FORCE_INLINE const Vec4f Vec4f_Load_Half2(const Half2& src)
{
#ifdef NFE_USE_FP16C
    uint32 value;
    memcpy(&value, &src, sizeof(Half2));
    return _mm_cvtph_ps(_mm_set1_epi32(value));
#else // NFE_USE_FP16C
    return Vec4f(src.x.ToFloat(), src.y.ToFloat(), 0.0f, 0.0f);
#endif // NFE_USE_FP16C
}

NFE_FORCE_INLINE const Vec4f Vec4f_Load_Half2(const Half2* src)
{
    return Vec4f_Load_Half2(*src);
}

NFE_FORCE_INLINE const Vec4f Vec4f_Load_Half4(const Half4& src)
{
#ifdef NFE_USE_FP16C
#if defined(NFE_ARCH_X64)
    uint64 value;
    memcpy(&value, &src, sizeof(Half4));
    return _mm_cvtph_ps(_mm_cvtsi64_si128(value));
#elif defined(NFE_ARCH_X86)
    return _mm_cvtph_ps(_mm_set_epi64x(0, src.packed));
#endif
#else // NFE_USE_FP16C
    return Vec4f(src.x.ToFloat(), src.y.ToFloat(), src.z.ToFloat(), src.w.ToFloat());
#endif // NFE_USE_FP16C
}

NFE_FORCE_INLINE const Vec4f Vec4f_Load_Half4(const Half4* src)
{
    return Vec4f_Load_Half4(*src);
}

// Convert uint16 (B5G6R5 format) to a Vec4f (normalized range)
NFE_FORCE_INLINE const Vec4f LoadVec4fUNorm(const Packed_5_6_5& src)
{
    const Vec4i mask{ 0x1F << 11, 0x3F << 5, 0x1F, 0 };
    const Vec4f scale{ 1.0f / 2048.0f / 31.0f, 1.0f / 32.0f / 63.0f, 1.0f / 31.0f, 0.0f };
    const Vec4i raw = Vec4i(src.v) & mask;
    return raw.ConvertToVec4f() * scale;
}

// Convert uint16 (B4G4R4A4 format) to a Vec4f (normalized range)
NFE_FORCE_INLINE const Vec4f LoadVec4fUNorm(const Packed_4_4_4_4& src)
{
    const Vec4i mask{ 0x0F00u, 0x00F0u, 0x000Fu, 0xF000u };
    const Vec4f scale{ 1.0f / 256.0f / 15.0f, 1.0f / 16.0f / 15.0f, 1.0f / 15.0f, 1.0f / 4096.0f / 15.0f };
    const Vec4i raw = Vec4i(src.v) & mask;
    return raw.ConvertToVec4f() * scale;
}

NFE_FORCE_INLINE const Vec4f LoadVec4fUNorm(const Packed_10_10_10_2& src)
{
#ifdef NFE_USE_SSE
    const Vec4f mask{ 0x3FFu, 0x3FFu << 10, 0x3FFu << 20, 0x3u << 30 };
    const Vec4f scale{ 1.0f / 1023.0f, 1.0f / (1023.0f * 1024.0f), 1.0f / (1023.0f * 1024.0f * 1024.0f), 1.0f / (3.0f * 1024.0f * 1024.0f * 1024.0f) };
    const Vec4f unsignedOffset{ 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const float*)&src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, scale);
#else
    return Vec4f
    {
        static_cast<float>(src.x) / 1023.0f,
        static_cast<float>(src.y) / 1023.0f,
        static_cast<float>(src.z) / 1023.0f,
        static_cast<float>(src.w) / 3.0f,
    };
#endif // NFE_USE_SSE
}

// Convert 2 uint8 to a Vec4f (normalized range)
NFE_FORCE_INLINE const Vec4f Vec4f_Load_2xUint8_Norm(const uint8* src)
{
#ifdef NFE_USE_SSE
    const Vec4f mask{ 0xFFu, 0xFF00u, 0u, 0u };
    const Vec4f scale{ 1.0f / 255.0f, 1.0f / (256.0f * 255.0f), 0.0f, 0.0f };
    const Vec4f unsignedOffset{ 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, scale);
#else
    return Vec4f
    {
        static_cast<float>(src[0]) / 255.0f,
        static_cast<float>(src[1]) / 255.0f,
    };
#endif // NFE_USE_SSE
}

// Convert 4 uint8 to a Vec4f
NFE_FORCE_INLINE const Vec4f Vec4f_Load_4xUint8(const uint8* src)
{
#ifdef NFE_USE_SSE
    const Vec4f mask{ 0xFFu, 0xFF00u, 0xFF0000u, 0xFF000000u };
    const Vec4f scale{ 1.0f, 1.0f / 256.0f, 1.0f / 65536.0f, 1.0f / (65536.0f * 256.0f) };
    const Vec4f unsignedOffset{ 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, scale);
#else
    return Vec4f
    {
        static_cast<float>(src[0]),
        static_cast<float>(src[1]),
        static_cast<float>(src[2]),
        static_cast<float>(src[3]),
    };
#endif // NFE_USE_SSE
}

// Convert 2 uint16 to a Vec4f (normalized range)
NFE_FORCE_INLINE const Vec4f Vec4f_Load_2xUint16_Norm(const uint16* src)
{
#ifdef NFE_USE_SSE
    const Vec4f maskX16Y16{ 0x0000FFFFu, 0xFFFF0000u, 0u, 0u };
    const Vec4f flipY{ 0u, 0x80000000u, 0u, 0u };
    const Vec4f fixUpY16{ 1.0f / 65535.0f, 1.0f / (65535.0f * 65536.0f), 0.0f, 0.0f };
    const Vec4f fixAddY16{ 0.0f, 32768.0f * 65536.0f, 0.0f, 0.0f };

    __m128 vTemp = _mm_load_ps1(reinterpret_cast<const float *>(src));
    // Mask x&0xFFFF, y&0xFFFF0000,z&0,w&0
    vTemp = _mm_and_ps(vTemp, maskX16Y16);
    // y needs to be sign flipped
    vTemp = _mm_xor_ps(vTemp, flipY);
    // Convert to floating point numbers
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    // y + 0x8000 to undo the signed order.
    vTemp = _mm_add_ps(vTemp, fixAddY16);
    // Y is 65536 times too large
    return _mm_mul_ps(vTemp, fixUpY16);
#else
    return Vec4f
    {
        static_cast<float>(src[0]) / 65535.0f,
        static_cast<float>(src[1]) / 65535.0f,
    };
#endif // NFE_USE_SSE
}

// Convert 4 uint16 to a Vec4f
NFE_FORCE_INLINE const Vec4f Vec4f_Load_4xUint16(const uint16* src)
{
#ifdef NFE_USE_SSE
    const Vec4f maskX16Y16Z16W16{ 0x0000FFFFu, 0x0000FFFFu, 0xFFFF0000u, 0xFFFF0000u };
    const __m128i shufflePattern = _mm_set_epi8(3, 2, 15, 14, 3, 2, 5, 4, 3, 2, 11, 10, 3, 2, 1, 0);
    // XXYYZZWWXXYYZZWW
    __m128d vIntd = _mm_load1_pd(reinterpret_cast<const double *>(src));
    // XX--ZZ----YY--WW
    __m128 masked = _mm_and_ps(_mm_castpd_ps(vIntd), maskX16Y16Z16W16);
    // --WW--ZZ--YY--XX
    __m128i reordered = _mm_shuffle_epi8(_mm_castps_si128(masked), shufflePattern);
    return _mm_cvtepi32_ps(reordered);
#else
    return Vec4f
    {
        static_cast<float>(src[0]),
        static_cast<float>(src[1]),
        static_cast<float>(src[2]),
        static_cast<float>(src[3]),
    };
#endif // NFE_USE_SSE
}

// Convert 3 uint8 to a Vec4f and scale to 0...1 range
NFE_FORCE_INLINE const Vec4f Vec4f_LoadRGB_UNorm(const uint8* src)
{
#ifdef NFE_USE_SSE
    const Vec4f mask{ 0xFFu, 0xFF00u, 0xFF0000u, 0x0u };
    const Vec4f scale{ 1.0f / 255.0f, 1.0f / 256.0f / 255.0f, 1.0f / 65536.0f / 255.0f, 0.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    return _mm_mul_ps(vTemp, scale);
#else
    return Vec4f
    {
        static_cast<float>(src[0]) / 255.0f,
        static_cast<float>(src[1]) / 255.0f,
        static_cast<float>(src[2]) / 255.0f,
        1.0f,
    };
#endif // NFE_USE_SSE
}

// Convert 3 uint8 to a Vec4f and scale to 0...1 range
NFE_FORCE_INLINE const Vec4f Vec4f_LoadBGR_UNorm(const uint8* src)
{
#ifdef NFE_USE_SSE
    const Vec4f mask{ 0xFF0000u, 0xFF00u, 0xFFu, 0x0u };
    const Vec4f scale{ 1.0f / 65536.0f / 255.0f, 1.0f / 256.0f / 255.0f, 1.0f / 255.0f, 0.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    return _mm_mul_ps(vTemp, scale);
#else
    return Vec4f
    {
        static_cast<float>(src[2]) / 255.0f,
        static_cast<float>(src[1]) / 255.0f,
        static_cast<float>(src[0]) / 255.0f,
        1.0f,
    };
#endif // NFE_USE_SSE
}

NFE_INLINE const Vec4f LoadVec4f(const PackedUnitVector3& src)
{
    Vec4f f = Vec4f::FromIntegers(src.u, src.v, 0, 0) * (1.0f / PackedUnitVector3::Scale);

    // based on: https://twitter.com/Stubbesaurus/status/937994790553227264

    const Vec4f fAbs = Vec4f::Abs(f);
    f.z = 1.0f - fAbs.x - fAbs.y;

    // t = Max([-f.z, -f.z, 0, 0], 0)
    const Vec4f t = Vec4f::Max(-f.Swizzle<2, 2, 3, 3>(), Vec4f::Zero());

    f += t.ChangeSign(f > Vec4f::Zero());

    return f.Normalized3();
}

NFE_INLINE const Vec4f LoadVec4f(const PackedColorRgbHdr& src)
{
    const Vec4f cocg = Vec4f::FromIntegers(src.co, src.cg, 0, 0) * (1.0f / PackedColorRgbHdr::ChromaScale);
    const float tmp = 1.0f - cocg.y;
    return Vec4f::Max(Vec4f::Zero(), Vec4f(tmp + cocg.x, 1.0f + cocg.y, tmp - cocg.x) * src.y);
}

NFE_INLINE const Vec4f LoadVec4f(const PackedUFloat3_11_11_10& src)
{
    uint32 x = ((src.xe + 112) << 23) | (src.xm << 17);
    uint32 y = ((src.ye + 112) << 23) | (src.ym << 17);
    uint32 z = ((src.ze + 112) << 23) | (src.zm << 17);
    return Vec4f(x, y, z, 0u);
}

NFE_INLINE const Vec4f LoadVec4f(const PackedUFloat3_9_9_9_5& src)
{
    const Vec4ui vInput(src.v);

    const Vec4ui mantissaShift(0, 9, 18, 0);
    const Vec4ui mantissaMask(0x1FF, 0x1FF, 0x1FF, 0);
    const Vec4i mantissa((vInput >> mantissaShift) & mantissaMask);

    const Vec4i exponent(vInput >> 27);
    const Vec4i base = Vec4i(0x33800000) + (exponent << 23);

    return base.AsVec4f() * mantissa.ConvertToVec4f();
}


} // namespace Math
} // namespace NFE
