#pragma once

#include "Vec8f.hpp"

namespace NFE {
namespace Math {

VecBool8i::VecBool8i(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
{
    v = _mm256_set_epi32(
        e7 ? -1 : 0,
        e6 ? -1 : 0,
        e5 ? -1 : 0,
        e4 ? -1 : 0,
        e3 ? -1 : 0,
        e2 ? -1 : 0,
        e1 ? -1 : 0,
        e0 ? -1 : 0
    );
}

template<uint32 index>
bool VecBool8i::Get() const
{
    static_assert(index < 8, "Invalid index");
    return _mm256_extract_epi32(v, index) != 0;
}

bool VecBool8i::All() const
{
    return _mm256_testc_si256(v, _mm256_set1_epi32(-1));
}

bool VecBool8i::None() const
{
    return _mm256_testz_si256(v, v) != 0;
}

bool VecBool8i::Any() const
{
    return _mm256_testz_si256(v, v) == 0;
}

const VecBool8i VecBool8i::operator & (const VecBool8i rhs) const
{
    return _mm256_and_si256(v, rhs.v);
}

const VecBool8i VecBool8i::operator | (const VecBool8i rhs) const
{
    return _mm256_or_si256(v, rhs.v);
}

const VecBool8i VecBool8i::operator ^ (const VecBool8i rhs) const
{
    return _mm256_xor_si256(v, rhs.v);
}

bool VecBool8i::operator == (const VecBool8i& other) const
{
    return _mm256_testc_si256(v, other.v) != 0;
}

bool VecBool8i::operator != (const VecBool8i& other) const
{
    return _mm256_testc_si256(v, other.v) == 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec8i Vec8i::Zero()
{
    return _mm256_setzero_si256();
}

Vec8i::Vec8i(const Vec8i& other)
    : v(other.v)
{}

Vec8i& Vec8i::operator = (const Vec8i& other)
{
    v = other.v;
    return *this;
}

Vec8i::Vec8i(const __m256i& m)
    : v(m)
{}

Vec8i::Vec8i(const __m256& m)
    : f(m)
{}

Vec8i::Vec8i(const Vec4i& lo, const Vec4i& hi)
    : v(_mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1u))
{}

const Vec8i Vec8i::Cast(const Vec8f& v)
{
    return _mm256_castps_si256(v);
}

const Vec8f Vec8i::AsVec8f() const
{
    return _mm256_castsi256_ps(v);
}

Vec8i::Vec8i(const int32 e0, const int32 e1, const int32 e2, const int32 e3, const int32 e4, const int32 e5, const int32 e6, const int32 e7)
    : v(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0))
{}

Vec8i::Vec8i(const int32 i)
    : v(_mm256_set1_epi32(i))
{}

Vec8i::Vec8i(const uint32 u)
    : v(_mm256_set1_epi32(u))
{}

const Vec8i Vec8i::SelectBySign(const Vec8i& a, const Vec8i& b, const VecBool8i& sel)
{
    return Vec8i(_mm256_blendv_ps(a.f, b.f, sel));
}

const Vec8i Vec8i::operator & (const Vec8i& b) const
{
    return Vec8i(_mm256_and_si256(v, b.v));
}

const Vec8i Vec8i::operator | (const Vec8i& b) const
{
    return Vec8i(_mm256_or_si256(v, b.v));
}

const Vec8i Vec8i::operator ^ (const Vec8i& b) const
{
    return Vec8i(_mm256_xor_si256(v, b.v));
}

Vec8i& Vec8i::operator &= (const Vec8i& b)
{
    v = _mm256_and_si256(v, b.v);
    return *this;
}

Vec8i& Vec8i::operator |= (const Vec8i& b)
{
    v = _mm256_or_si256(v, b.v);
    return *this;
}

Vec8i& Vec8i::operator ^= (const Vec8i& b)
{
    v = _mm256_xor_si256(v, b.v);
    return *this;
}

const Vec8i Vec8i::Convert(const Vec8f& v)
{
    return _mm256_cvtps_epi32(_mm256_round_ps(v, _MM_FROUND_TO_ZERO));
}

const Vec8f Vec8i::ConvertToVec8f() const
{
    return _mm256_cvtepi32_ps(v);
}

const Vec8i Vec8i::operator - () const
{
    return Vec8i::Zero() - (*this);
}

const Vec8i Vec8i::operator + (const Vec8i& b) const
{
    return _mm256_add_epi32(v, b);
}

const Vec8i Vec8i::operator - (const Vec8i& b) const
{
    return _mm256_sub_epi32(v, b);
}

const Vec8i Vec8i::operator * (const Vec8i& b) const
{
    return _mm256_mullo_epi32(v, b);
}

Vec8i& Vec8i::operator += (const Vec8i& b)
{
    v = _mm256_add_epi32(v, b);
    return *this;
}

Vec8i& Vec8i::operator -= (const Vec8i& b)
{
    v = _mm256_sub_epi32(v, b);
    return *this;
}

Vec8i& Vec8i::operator *= (const Vec8i& b)
{
    v = _mm256_mullo_epi32(v, b);
    return *this;
}

const Vec8i Vec8i::operator + (int32 b) const
{
    return _mm256_add_epi32(v, _mm256_set1_epi32(b));
}

const Vec8i Vec8i::operator - (int32 b) const
{
    return _mm256_sub_epi32(v, _mm256_set1_epi32(b));
}

const Vec8i Vec8i::operator * (int32 b) const
{
    return _mm256_mullo_epi32(v, _mm256_set1_epi32(b));
}

const Vec8i Vec8i::operator % (int32 b) const
{
    // TODO
    return Vec8i(i[0] % b, i[1] % b, i[2] % b, i[3] % b, i[4] % b, i[5] % b, i[6] % b, i[7] % b);
}

Vec8i& Vec8i::operator += (int32 b)
{
    v = _mm256_add_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

Vec8i& Vec8i::operator -= (int32 b)
{
    v = _mm256_sub_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

Vec8i& Vec8i::operator *= (int32 b)
{
    v = _mm256_mullo_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VecBool8i Vec8i::operator == (const Vec8i& b) const
{
    return _mm256_cmpeq_epi32(v, b.v);
}

const VecBool8i Vec8i::operator != (const Vec8i& b) const
{
    // TODO may not be optimal, when used with selector later on
    // idea: have VecNegBool or something
    return _mm256_xor_si256(_mm256_set1_epi32(-1), _mm256_cmpeq_epi32(v, b.v));
}

const Vec8i Vec8i::operator << (const Vec8i& b) const
{
    return _mm256_sllv_epi32(v, b);
}

const Vec8i Vec8i::operator >> (const Vec8i& b) const
{
    return _mm256_srlv_epi32(v, b);
}

const Vec8i Vec8i::operator << (int32 b) const
{
    return _mm256_slli_epi32(v, b);
}

const Vec8i Vec8i::operator >> (int32 b) const
{
    return _mm256_srli_epi32(v, b);
}

const Vec8i Vec8i::Min(const Vec8i& a, const Vec8i& b)
{
    return _mm256_min_epi32(a, b);
}

const Vec8i Vec8i::Max(const Vec8i& a, const Vec8i& b)
{
    return _mm256_max_epi32(a, b);
}

const Vec8f Gather8(const float* basePtr, const Vec8i& indices)
{
    return _mm256_i32gather_ps(basePtr, indices, 4);
}

} // namespace Math
} // namespace NFE
