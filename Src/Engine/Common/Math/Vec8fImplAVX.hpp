#pragma once

namespace NFE {
namespace Math {

VecBool8f::VecBool8f(bool scalar)
{
#ifdef NFE_USE_AVX512
    mask = _cvtu32_mask8(scalar ? 0xFFFF : 0);
#else
    v = _mm256_castsi256_ps(_mm256_set1_epi32(scalar ? 0xFFFFFFFF : 0));
#endif
}

VecBool8f::VecBool8f(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
{
#ifdef NFE_USE_AVX512
    mask = _cvtu32_mask8(uint16(e0) | e1<<1 | e2<<2 | e3<<3 | e4<<4 | e5<<5 | e6<<6 | e7<<7);
#else
    v = _mm256_castsi256_ps(_mm256_set_epi32(
        e7 ? 0xFFFFFFFF : 0,
        e6 ? 0xFFFFFFFF : 0,
        e5 ? 0xFFFFFFFF : 0,
        e4 ? 0xFFFFFFFF : 0,
        e3 ? 0xFFFFFFFF : 0,
        e2 ? 0xFFFFFFFF : 0,
        e1 ? 0xFFFFFFFF : 0,
        e0 ? 0xFFFFFFFF : 0
    ));
#endif
}

template<uint32 index>
bool VecBool8f::Get() const
{
    static_assert(index < 8, "Invalid index");
#ifdef NFE_USE_AVX512
    return (uint32(mask) & (1 << index)) != 0;
#else
    return _mm256_extract_epi32(_mm256_castps_si256(v), index) != 0;
#endif
}

uint32 VecBool8f::GetMask() const
{
#ifdef NFE_USE_AVX512
    return mask;
#else
    return (uint32)_mm256_movemask_ps(v);
#endif
}

bool VecBool8f::All() const
{
    return GetMask() == 0xFF;
}

bool VecBool8f::None() const
{
#ifdef NFE_USE_AVX512
    return _mm512_testz_or_mask8(mask, mask) != 0;
#else
    return _mm256_movemask_ps(v) == 0;
#endif
}

bool VecBool8f::Any() const
{
#ifdef NFE_USE_AVX512
    return _mm512_testz_or_mask8(mask, mask) == 0;
#else
    return _mm256_movemask_ps(v) != 0;
#endif
}

const VecBool8f VecBool8f::operator & (const VecBool8f rhs) const
{
#ifdef NFE_USE_AVX512
    return _kand_mask8(mask, rhs.mask);
#else
    return _mm256_and_ps(v, rhs.v);
#endif
}

const VecBool8f VecBool8f::operator | (const VecBool8f rhs) const
{
#ifdef NFE_USE_AVX512
    return _kor_mask8(mask, rhs.mask);
#else
    return _mm256_or_ps(v, rhs.v);
#endif
}

const VecBool8f VecBool8f::operator ^ (const VecBool8f rhs) const
{
#ifdef NFE_USE_AVX512
    return _kxor_mask8(mask, rhs.mask);
#else
    return _mm256_xor_ps(v, rhs.v);
#endif
}

bool VecBool8f::operator == (const VecBool8f rhs) const
{
    return GetMask() == rhs.GetMask();
}

// Constructors ===================================================================================

Vec8f::Vec8f() = default;

const Vec8f Vec8f::Zero()
{
    return _mm256_setzero_ps();
}

Vec8f::Vec8f(const Vec8f& other)
    : v(other.v)
{}

Vec8f::Vec8f(const Vec4f& lo)
    : v(_mm256_castps128_ps256(lo))
{}

Vec8f::Vec8f(const Vec4f& lo, const Vec4f& hi)
    : v(_mm256_insertf128_ps(_mm256_castps128_ps256(lo), hi, 1))
{}

Vec8f::Vec8f(const __m256& m)
    : v(m)
{}

Vec8f::Vec8f(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7)
    : v(_mm256_set_ps(e7, e6, e5, e4, e3, e2, e1, e0))
{}

Vec8f::Vec8f(int32 e0, int32 e1, int32 e2, int32 e3, int32 e4, int32 e5, int32 e6, int32 e7)
    : v(_mm256_castsi256_ps(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0)))
{}

Vec8f::Vec8f(uint32 e0, uint32 e1, uint32 e2, uint32 e3, uint32 e4, uint32 e5, uint32 e6, uint32 e7)
    : v(_mm256_castsi256_ps(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0)))
{}

Vec8f::Vec8f(const float* src)
    : v(_mm256_loadu_ps(src))
{}

Vec8f::Vec8f(const float scalar)
    : v(_mm256_set1_ps(scalar))
{}

Vec8f::Vec8f(const int32 i)
    : v(_mm256_castsi256_ps(_mm256_set1_epi32(i)))
{}

Vec8f::Vec8f(const uint32 u)
    : v(_mm256_castsi256_ps(_mm256_set1_epi32(u)))
{}

const Vec8f Vec8f::FromInteger(int32 x)
{
    return _mm256_cvtepi32_ps(_mm256_set1_epi32(x));
}

Vec8f& Vec8f::operator = (const Vec8f& other)
{
    v = other.v;
    return *this;
}

const Vec8f Vec8f::Select(const Vec8f& a, const Vec8f& b, const VecBool8f& sel)
{
#if defined(NFE_USE_AVX512)
    return _mm256_mask_blend_ps(sel, a, b);
#else
    return _mm256_blendv_ps(a, b, sel.v);
#endif
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vec8f Vec8f::Select(const Vec8f& a, const Vec8f& b)
{
    static_assert(selX <= 1, "Invalid X index");
    static_assert(selY <= 1, "Invalid Y index");
    static_assert(selZ <= 1, "Invalid Z index");
    static_assert(selW <= 1, "Invalid W index");

    constexpr uint32 maskLow = selX | (selY << 1) | (selZ << 2) | (selW << 3);
    return _mm256_blend_ps(a, b, maskLow | (maskLow << 4));
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec8f Vec8f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return _mm256_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

// extract lower lanes
const Vec4f Vec8f::Low() const
{
    return Vec4f(_mm256_extractf128_ps(v, 0));
}

// extract higher lanes
const Vec4f Vec8f::High() const
{
    return Vec4f(_mm256_extractf128_ps(v, 1));
}

// Logical operations =============================================================================

const Vec8f Vec8f::operator& (const Vec8f& b) const
{
    return _mm256_and_ps(v, b);
}

const Vec8f Vec8f::operator| (const Vec8f& b) const
{
    return _mm256_or_ps(v, b);
}

const Vec8f Vec8f::operator^ (const Vec8f& b) const
{
    return _mm256_xor_ps(v, b);
}

Vec8f& Vec8f::operator&= (const Vec8f& b)
{
    v = _mm256_and_ps(v, b);
    return *this;
}

Vec8f& Vec8f::operator|= (const Vec8f& b)
{
    v = _mm256_or_ps(v, b);
    return *this;
}

Vec8f& Vec8f::operator^= (const Vec8f& b)
{
    v = _mm256_xor_ps(v, b);
    return *this;
}

// Simple arithmetics =============================================================================

const Vec8f Vec8f::operator- () const
{
    return Vec8f::Zero() - (*this);
}

const Vec8f Vec8f::operator+ (const Vec8f& b) const
{
    return _mm256_add_ps(v, b);
}

const Vec8f Vec8f::operator- (const Vec8f& b) const
{
    return _mm256_sub_ps(v, b);
}

const Vec8f Vec8f::operator* (const Vec8f& b) const
{
    return _mm256_mul_ps(v, b);
}

const Vec8f Vec8f::operator/ (const Vec8f& b) const
{
    return _mm256_div_ps(v, b);
}

const Vec8f Vec8f::operator* (float b) const
{
    return _mm256_mul_ps(v, _mm256_set1_ps(b));
}

const Vec8f Vec8f::operator/ (float b) const
{
    return _mm256_div_ps(v, _mm256_set1_ps(b));
}

const Vec8f operator*(float a, const Vec8f& b)
{
    return _mm256_mul_ps(b, _mm256_set1_ps(a));
}


Vec8f& Vec8f::operator+= (const Vec8f& b)
{
    v = _mm256_add_ps(v, b);
    return *this;
}

Vec8f& Vec8f::operator-= (const Vec8f& b)
{
    v = _mm256_sub_ps(v, b);
    return *this;
}

Vec8f& Vec8f::operator*= (const Vec8f& b)
{
    v = _mm256_mul_ps(v, b);
    return *this;
}

Vec8f& Vec8f::operator/= (const Vec8f& b)
{
    v = _mm256_div_ps(v, b);
    return *this;
}

Vec8f& Vec8f::operator*= (float b)
{
    v = _mm256_mul_ps(v, _mm256_set1_ps(b));
    return *this;
}

Vec8f& Vec8f::operator/= (float b)
{
    v = _mm256_div_ps(v, _mm256_set1_ps(b));
    return *this;
}

const Vec8f Vec8f::MulAndAdd(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
#ifdef NFE_USE_FMA
    return _mm256_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

const Vec8f Vec8f::MulAndSub(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
#ifdef NFE_USE_FMA
    return _mm256_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

const Vec8f Vec8f::NegMulAndAdd(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
#ifdef NFE_USE_FMA
    return _mm256_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

const Vec8f Vec8f::NegMulAndSub(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
#ifdef NFE_USE_FMA
    return _mm256_fnmsub_ps(a, b, c);
#else
    return c - a * b;
#endif
}

const Vec8f Vec8f::Floor(const Vec8f& V)
{
    Vec8f vResult = _mm256_sub_ps(V, _mm256_set1_ps(0.49999f));
    __m256i vInt = _mm256_cvtps_epi32(vResult);
    vResult = _mm256_cvtepi32_ps(vInt);
    return vResult;
}

const Vec8f Vec8f::Sqrt(const Vec8f& V)
{
    return _mm256_sqrt_ps(V);
}

const Vec8f Vec8f::Reciprocal(const Vec8f& V)
{
    return _mm256_div_ps(_mm256_set1_ps(1.0f), V);
}

const Vec8f Vec8f::FastReciprocal(const Vec8f& v)
{
    const __m256 rcp = _mm256_rcp_ps(v);
    const __m256 rcpSqr = _mm256_mul_ps(rcp, rcp);
    const __m256 rcp2 = _mm256_add_ps(rcp, rcp);
    return NegMulAndAdd(rcpSqr, v, rcp2);
}

const Vec8f Vec8f::Min(const Vec8f& a, const Vec8f& b)
{
    return _mm256_min_ps(a, b);
}

const Vec8f Vec8f::Max(const Vec8f& a, const Vec8f& b)
{
    return _mm256_max_ps(a, b);
}

const Vec8f Vec8f::Abs(const Vec8f& v)
{
    return _mm256_and_ps(v, VECTOR8_MASK_ABS);
}

uint32 Vec8f::GetSignMask() const
{
    return static_cast<uint32>(_mm256_movemask_ps(v));
}

const Vec8f Vec8f::HorizontalMax() const
{
    __m256 temp;
    temp = _mm256_max_ps(v, _mm256_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm256_max_ps(temp, _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    temp = _mm256_max_ps(temp, _mm256_permute2f128_ps(temp, temp, 1));
    return temp;
}

const Vec8f Vec8f::Fmod1(const Vec8f& x)
{
    return _mm256_sub_ps(x, _mm256_round_ps(x, _MM_FROUND_TO_ZERO));
}

/*
void Vec8f::Transpose8x8(Vec8f& v0, Vec8f& v1, Vec8f& v2, Vec8f& v3, Vec8f& v4, Vec8f& v5, Vec8f& v6, Vec8f& v7)
{
    const __m256 t0 = _mm256_unpacklo_ps(v0, v1);
    const __m256 t1 = _mm256_unpackhi_ps(v0, v1);
    const __m256 t2 = _mm256_unpacklo_ps(v2, v3);
    const __m256 t3 = _mm256_unpackhi_ps(v2, v3);
    const __m256 t4 = _mm256_unpacklo_ps(v4, v5);
    const __m256 t5 = _mm256_unpackhi_ps(v4, v5);
    const __m256 t6 = _mm256_unpacklo_ps(v6, v7);
    const __m256 t7 = _mm256_unpackhi_ps(v6, v7);

    // Using 4 shuffles + 8 blends (12 instructions in total) is faster than only 8 shuffles
    // blends can be executed in parallel with shuffles, while shuffle can be only executed at port 5
    __m256 v;
    v = _mm256_shuffle_ps(t0, t2, 0x4E);
    const __m256 tt0 = _mm256_blend_ps(t0, v, 0xCC);
    const __m256 tt1 = _mm256_blend_ps(t2, v, 0x33);
    v = _mm256_shuffle_ps(t1, t3, 0x4E);
    const __m256 tt2 = _mm256_blend_ps(t1, v, 0xCC);
    const __m256 tt3 = _mm256_blend_ps(t3, v, 0x33);
    v = _mm256_shuffle_ps(t4, t6, 0x4E);
    const __m256 tt4 = _mm256_blend_ps(t4, v, 0xCC);
    const __m256 tt5 = _mm256_blend_ps(t6, v, 0x33);
    v = _mm256_shuffle_ps(t5, t7, 0x4E);
    const __m256 tt6 = _mm256_blend_ps(t5, v, 0xCC);
    const __m256 tt7 = _mm256_blend_ps(t7, v, 0x33);

    //const __m256 tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
    //const __m256 tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt3 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2));
    //const __m256 tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
    //const __m256 tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt7 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(3, 2, 3, 2));

    v0 = _mm256_permute2f128_ps(tt0, tt4, 0x20);
    v1 = _mm256_permute2f128_ps(tt1, tt5, 0x20);
    v2 = _mm256_permute2f128_ps(tt2, tt6, 0x20);
    v3 = _mm256_permute2f128_ps(tt3, tt7, 0x20);
    v4 = _mm256_permute2f128_ps(tt0, tt4, 0x31);
    v5 = _mm256_permute2f128_ps(tt1, tt5, 0x31);
    v6 = _mm256_permute2f128_ps(tt2, tt6, 0x31);
    v7 = _mm256_permute2f128_ps(tt3, tt7, 0x31);
}
*/

// Comparison functions ===========================================================================

const VecBool8f Vec8f::operator == (const Vec8f& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_ps_mask(v, b.v, _CMP_EQ_OQ);
#else
    return _mm256_cmp_ps(v, b.v, _CMP_EQ_OQ);
#endif
}

const VecBool8f Vec8f::operator < (const Vec8f& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_ps_mask(v, b.v, _CMP_LT_OQ);
#else
    return _mm256_cmp_ps(v, b.v, _CMP_LT_OQ);
#endif
}

const VecBool8f Vec8f::operator <= (const Vec8f& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_ps_mask(v, b.v, _CMP_LE_OQ);
#else
    return _mm256_cmp_ps(v, b.v, _CMP_LE_OQ);
#endif
}

const VecBool8f Vec8f::operator > (const Vec8f& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_ps_mask(v, b.v, _CMP_GT_OQ);
#else
    return _mm256_cmp_ps(v, b.v, _CMP_GT_OQ);
#endif
}

const VecBool8f Vec8f::operator >= (const Vec8f& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_ps_mask(v, b.v, _CMP_GE_OQ);
#else
    return _mm256_cmp_ps(v, b.v, _CMP_GE_OQ);
#endif
}

const VecBool8f Vec8f::operator != (const Vec8f& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_ps_mask(v, b.v, _CMP_NEQ_OQ);
#else
    return _mm256_cmp_ps(v, b.v, _CMP_NEQ_OQ);
#endif
}

bool Vec8f::IsZero() const
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v, _mm256_setzero_ps(), _CMP_EQ_OQ)) == 0xFF;
}

bool Vec8f::IsNaN() const
{
    // Test against itself. NaN is always not equal
    const __m256 temp = _mm256_cmp_ps(v, v, _CMP_NEQ_OQ);
    return _mm256_movemask_ps(temp) != 0;
}

bool Vec8f::IsInfinite() const
{
    // Mask off the sign bit
    __m256 temp = _mm256_and_ps(v, VECTOR8_MASK_ABS);
    // Compare to infinity
    temp = _mm256_cmp_ps(temp, VECTOR8_INF, _CMP_EQ_OQ);
    return _mm256_movemask_ps(temp) != 0;
}

} // namespace Math
} // namespace NFE
