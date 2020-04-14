#pragma once

namespace NFE {
namespace Math {

VecBool16::VecBool16(bool scalar)
{
    mask = _cvtu32_mask16(scalar ? 0xFFFF : 0);
}

VecBool16::VecBool16(
    bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7,
    bool e8, bool e9, bool e10, bool e11, bool e12, bool e13, bool e14, bool e15)
{
    mask = _cvtu32_mask16(
        uint16(e0) | e1<<1 | e2<<2 | e3<<3 | e4<<4 | e5<<5 | e6<<6 | e7<<7 |
        e8<<8 | e9<<9 | e10<<10 | e11<<11 | e12<<12 | e13<<13 | e14<<15 | e15<<15
    );
}

template<uint32 index>
bool VecBool16::Get() const
{
    static_assert(index < 16, "Invalid index");
    return (uint32(mask) & (1 << index)) != 0;
}

uint16 VecBool16::GetMask() const
{
    return mask;
}

bool VecBool16::All() const
{
    return GetMask() == 0xFFFF;
}

bool VecBool16::None() const
{
    return _mm512_testz_or_mask16(mask, mask) != 0;
}

bool VecBool16::Any() const
{
    return _mm512_testz_or_mask16(mask, mask) == 0;
}

const VecBool16 VecBool16::operator & (const VecBool16 rhs) const
{
    return _kand_mask16(mask, rhs.mask);
}

const VecBool16 VecBool16::operator | (const VecBool16 rhs) const
{
    return _kor_mask16(mask, rhs.mask);
}

const VecBool16 VecBool16::operator ^ (const VecBool16 rhs) const
{
    return _kxor_mask16(mask, rhs.mask);
}

bool VecBool16::operator == (const VecBool16 rhs) const
{
    return GetMask() == rhs.GetMask();
}

// Constructors ===================================================================================

Vec16f::Vec16f() = default;

const Vec16f Vec16f::Zero()
{
    return _mm512_setzero_ps();
}

Vec16f::Vec16f(const Vec16f& other)
    : v(other.v)
{}

Vec16f::Vec16f(const Vec8f& lo, const Vec8f& hi)
    : v(_mm512_insertf32x8(_mm512_castps256_ps512(lo), hi, 1))
{}

Vec16f::Vec16f(const __m512& m)
    : v(m)
{}

Vec16f::Vec16f(
    float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7,
    float e8, float e9, float e10, float e11, float e12, float e13, float e14, float e15)
    : v(_mm512_setr_ps(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15))
{}

Vec16f::Vec16f(const float* src)
    : v(_mm512_loadu_ps(src))
{}

Vec16f::Vec16f(const float scalar)
    : v(_mm512_set1_ps(scalar))
{}

Vec16f::Vec16f(const int32 i)
    : v(_mm512_castsi512_ps(_mm512_set1_epi32(i)))
{}

Vec16f::Vec16f(const uint32 u)
    : v(_mm512_castsi512_ps(_mm512_set1_epi32(u)))
{}

const Vec16f Vec16f::FromInteger(int32 x)
{
    return _mm512_cvtepi32_ps(_mm512_set1_epi32(x));
}

Vec16f& Vec16f::operator = (const Vec16f& other)
{
    v = other.v;
    return *this;
}

const Vec16f Vec16f::Select(const Vec16f& a, const Vec16f& b, const VecBool16& sel)
{
    return _mm512_mask_blend_ps(sel, a, b);
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vec16f Vec16f::Select(const Vec16f& a, const Vec16f& b)
{
    static_assert(selX <= 1, "Invalid X index");
    static_assert(selY <= 1, "Invalid Y index");
    static_assert(selZ <= 1, "Invalid Z index");
    static_assert(selW <= 1, "Invalid W index");

    constexpr uint32 maskLow = selX | (selY << 1) | (selZ << 2) | (selW << 3);
    return _mm512_blend_ps(a, b, maskLow | (maskLow << 4));
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec16f Vec16f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return _mm512_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

// extract lower lanes
const Vec8f Vec16f::Low() const
{
    return _mm512_castps512_ps256(v);
}

// extract higher lanes
const Vec8f Vec16f::High() const
{
    return _mm512_extractf32x8_ps(v, 1);
}

// Logical operations =============================================================================

const Vec16f Vec16f::operator & (const Vec16f& b) const
{
    return _mm512_and_ps(v, b);
}

const Vec16f Vec16f::operator | (const Vec16f& b) const
{
    return _mm512_or_ps(v, b);
}

const Vec16f Vec16f::operator ^ (const Vec16f& b) const
{
    return _mm512_xor_ps(v, b);
}

Vec16f& Vec16f::operator &= (const Vec16f& b)
{
    v = _mm512_and_ps(v, b);
    return *this;
}

Vec16f& Vec16f::operator |= (const Vec16f& b)
{
    v = _mm512_or_ps(v, b);
    return *this;
}

Vec16f& Vec16f::operator ^= (const Vec16f& b)
{
    v = _mm512_xor_ps(v, b);
    return *this;
}

// Simple arithmetics =============================================================================

const Vec16f Vec16f::operator - () const
{
    return Vec16f::Zero() - (*this);
}

const Vec16f Vec16f::operator + (const Vec16f& b) const
{
    return _mm512_add_ps(v, b);
}

const Vec16f Vec16f::operator - (const Vec16f& b) const
{
    return _mm512_sub_ps(v, b);
}

const Vec16f Vec16f::operator * (const Vec16f& b) const
{
    return _mm512_mul_ps(v, b);
}

const Vec16f Vec16f::operator / (const Vec16f& b) const
{
    return _mm512_div_ps(v, b);
}

const Vec16f Vec16f::operator * (float b) const
{
    return _mm512_mul_ps(v, _mm512_set1_ps(b));
}

const Vec16f Vec16f::operator / (float b) const
{
    return _mm512_div_ps(v, _mm512_set1_ps(b));
}

const Vec16f operator * (float a, const Vec16f& b)
{
    return _mm512_mul_ps(b, _mm512_set1_ps(a));
}


Vec16f& Vec16f::operator+= (const Vec16f& b)
{
    v = _mm512_add_ps(v, b);
    return *this;
}

Vec16f& Vec16f::operator-= (const Vec16f& b)
{
    v = _mm512_sub_ps(v, b);
    return *this;
}

Vec16f& Vec16f::operator*= (const Vec16f& b)
{
    v = _mm512_mul_ps(v, b);
    return *this;
}

Vec16f& Vec16f::operator/= (const Vec16f& b)
{
    v = _mm512_div_ps(v, b);
    return *this;
}

Vec16f& Vec16f::operator*= (float b)
{
    v = _mm512_mul_ps(v, _mm512_set1_ps(b));
    return *this;
}

Vec16f& Vec16f::operator/= (float b)
{
    v = _mm512_div_ps(v, _mm512_set1_ps(b));
    return *this;
}

const Vec16f Vec16f::MulAndAdd(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
#ifdef NFE_USE_FMA
    return _mm512_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

const Vec16f Vec16f::MulAndSub(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
#ifdef NFE_USE_FMA
    return _mm512_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

const Vec16f Vec16f::NegMulAndAdd(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
#ifdef NFE_USE_FMA
    return _mm512_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

const Vec16f Vec16f::NegMulAndSub(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
#ifdef NFE_USE_FMA
    return _mm512_fnmsub_ps(a, b, c);
#else
    return c - a * b;
#endif
}

const Vec16f Vec16f::Floor(const Vec16f& x)
{
    return _mm512_floor_ps(x);
}

const Vec16f Vec16f::Sqrt(const Vec16f& x)
{
    return _mm512_sqrt_ps(x);
}

const Vec16f Vec16f::Reciprocal(const Vec16f& x)
{
    return _mm512_div_ps(_mm512_set1_ps(1.0f), x);
}

const Vec16f Vec16f::FastReciprocal(const Vec16f& x)
{
    return _mm512_rcp14_ps(x);
}

const Vec16f Vec16f::Min(const Vec16f& a, const Vec16f& b)
{
    return _mm512_min_ps(a, b);
}

const Vec16f Vec16f::Max(const Vec16f& a, const Vec16f& b)
{
    return _mm512_max_ps(a, b);
}

const Vec16f Vec16f::Abs(const Vec16f& v)
{
    return _mm512_abs_ps(v);
}

uint32 Vec16f::GetSignMask() const
{
    return static_cast<uint32>(_mm512_movepi32_mask(_mm512_castps_si512(v)));
}

// Comparison functions ===========================================================================

const VecBool16 Vec16f::operator == (const Vec16f& b) const
{
    return _mm512_cmp_ps_mask(v, b.v, _CMP_EQ_OQ);
}

const VecBool16 Vec16f::operator < (const Vec16f& b) const
{
    return _mm512_cmp_ps_mask(v, b.v, _CMP_LT_OQ);
}

const VecBool16 Vec16f::operator <= (const Vec16f& b) const
{
    return _mm512_cmp_ps_mask(v, b.v, _CMP_LE_OQ);
}

const VecBool16 Vec16f::operator > (const Vec16f& b) const
{
    return _mm512_cmp_ps_mask(v, b.v, _CMP_GT_OQ);
}

const VecBool16 Vec16f::operator >= (const Vec16f& b) const
{
    return _mm512_cmp_ps_mask(v, b.v, _CMP_GE_OQ);
}

const VecBool16 Vec16f::operator != (const Vec16f& b) const
{
    return _mm512_cmp_ps_mask(v, b.v, _CMP_NEQ_OQ);
}

//bool Vec16f::IsZero() const
//{
//    return _mm512_movemask_ps(_mm512_cmp_ps(v, _mm512_setzero_ps(), _CMP_EQ_OQ)) == 0xFF;
//}
//
//bool Vec16f::IsNaN() const
//{
//    // Test against itself. NaN is always not equal
//    const __m256 temp = _mm512_cmp_ps(v, v, _CMP_NEQ_OQ);
//    return _mm512_movemask_ps(temp) != 0;
//}
//
//bool Vec16f::IsInfinite() const
//{
//    // Mask off the sign bit
//    __m256 temp = _mm512_and_ps(v, VECTOR8_MASK_ABS);
//    // Compare to infinity
//    temp = _mm512_cmp_ps(temp, VECTOR8_INF, _CMP_EQ_OQ);
//    return _mm512_movemask_ps(temp) != 0;
//}

} // namespace Math
} // namespace NFE
