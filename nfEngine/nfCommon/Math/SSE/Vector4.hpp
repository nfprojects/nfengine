/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  SSE version of Vector4 class definitions.
 */

#pragma once

#if !defined(NFE_USE_SSE)
#error "NFE_USE_SSE must be defined when including this header"
#endif

namespace NFE {
namespace Math {


static_assert(alignof(Vector4) == 4 * sizeof(float), "Invalid Vector4 alignment");


// Constructors ===================================================================================

Vector4::Vector4()
{
    v = _mm_setzero_ps();
}

Vector4::Vector4(float x, float y, float z, float w)
{
    v = _mm_set_ps(w, z, y, x);
}

Vector4::Vector4(int x, int y, int z, int w)
{
    v = _mm_castsi128_ps(_mm_set_epi32(w, z, y, x));
}

// copy array of 4 floats
Vector4::Vector4(const float* src)
{
    v = _mm_loadu_ps(src);
}

Vector4::Vector4(const Float2& src)
{
    __m128 vx = _mm_load_ss(&src.x);
    __m128 vy = _mm_load_ss(&src.y);
    v = _mm_unpacklo_ps(vx, vy);
}

Vector4::Vector4(const Float3& src)
{
    __m128 vx = _mm_load_ss(&src.x);
    __m128 vy = _mm_load_ss(&src.y);
    __m128 vz = _mm_load_ss(&src.z);
    __m128 vxy = _mm_unpacklo_ps(vx, vy);
    v = _mm_movelh_ps(vxy, vz);
}

Vector4::Vector4(const Float4& src)
{
    v = _mm_loadu_ps(&src.x);
}

void Vector4::Set(float scalar)
{
    v = _mm_set1_ps(scalar);
}

// Load & store ===================================================================================

Vector4 Vector4::Load4(const uint8* src)
{
    static const Vector4i mask = {{{0xFF, 0xFF00, 0xFF0000, 0xFF000000}}};
    static const __m128 LoadUByte4Mul = {1.0f, 1.0f / 256.0f, 1.0f / 65536.0f, 1.0f / (65536.0f * 256.0f)};
    static const Vector4i flipW = {{{0, 0, 0, 0x80000000}}};
    static const __m128 unsignedOffset = {0, 0, 0, 32768.0f * 65536.0f};

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, flipW.v);

    // convert to float
    vTemp = _mm_cvtepi32_ps(reinterpret_cast<const __m128i*>(&vTemp)[0]);
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, LoadUByte4Mul);
}

/**
 * Convert a Vector4 to 4 unsigned chars.
 */
void Vector4::Store4(uint8* dest) const
{
    static const __m128 MaxUByte4 = {255.0f, 255.0f, 255.0f, 255.0f};

    // Clamp
    __m128 vResult = _mm_max_ps(v, _mm_setzero_ps());
    vResult = _mm_min_ps(vResult, MaxUByte4);

    // Convert to int & extract components
    __m128i vResulti = _mm_cvttps_epi32(vResult);
    __m128i Yi = _mm_srli_si128(vResulti, 3);
    __m128i Zi = _mm_srli_si128(vResulti, 6);
    __m128i Wi = _mm_srli_si128(vResulti, 9);

    vResulti = _mm_or_si128(_mm_or_si128(Wi, Zi), _mm_or_si128(Yi, vResulti));
    _mm_store_ss(reinterpret_cast<float*>(dest), reinterpret_cast<const __m128*>(&vResulti)[0]);
}

void Vector4::Store(float* dest) const
{
    _mm_store_ss(dest, v);
}

void Vector4::Store(Float2* dest) const
{
    __m128 vy = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    _mm_store_ss(&dest->x, v);
    _mm_store_ss(&dest->y, vy);
}

void Vector4::Store(Float3* dest) const
{
    __m128 vy = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 vz = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    _mm_store_ss(&dest->x, v);
    _mm_store_ss(&dest->y, vy);
    _mm_store_ss(&dest->z, vz);
}

void Vector4::Store(Float4* dest) const
{
    _mm_storeu_ps(&dest->x, v);
}

template<bool negX, bool negY, bool negZ, bool negW>
Vector4 Vector4::ChangeSign() const
{
    if (!(negX || negY || negZ || negW))
    {
        // no negation
        return *this;
    }

    // generate bit negation mask
    static const Vector4i mask = { { { negX ? 0x80000000 : 0, negY ? 0x80000000 : 0, negZ ? 0x80000000 : 0, negW ? 0x80000000 : 0} } };

    // flip sign bits
    return _mm_xor_ps(v, _mm_castsi128_ps(mask));
}

// Elements rearrangement =========================================================================

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Vector4 Vector4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Vector4 Vector4::Blend(const Vector4& a, const Vector4& b)
{
    static_assert(ix < 2, "Invalid index for X component");
    static_assert(iy < 2, "Invalid index for Y component");
    static_assert(iz < 2, "Invalid index for Z component");
    static_assert(iw < 2, "Invalid index for W component");

#if defined(NFE_USE_SSE4)
    constexpr uint32 mask = ix | (iy << 1) | (iz << 2) | (iw << 3);
    return _mm_blend_ps(a, b, mask);
#else
    static const Vector4i mask = { { { ix ? 0xFFFFFFFF : 0, iy ? 0xFFFFFFFF : 0, iz ? 0xFFFFFFFF : 0, iw ? 0xFFFFFFFF : 0 } } };
    const Vector4 maskf = _mm_castsi128_ps(mask);
    return _mm_or_ps(_mm_andnot_ps(maskf, a), _mm_and_ps(maskf, b));
#endif
}

Vector4 Vector4::SplatX() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}

Vector4 Vector4::SplatY() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}

Vector4 Vector4::SplatZ() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}

Vector4 Vector4::SplatW() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

Vector4 Vector4::Splat(float f)
{
    return _mm_set_ps1(f);
}

Vector4 Vector4::SelectBySign(const Vector4& a, const Vector4& b, const Vector4& sel)
{
#if defined(NFE_USE_SSE4)
    return _mm_blendv_ps(a, b, sel);
#else
    Vector4 mask = _mm_cmplt_ps(sel, _mm_setzero_ps());
    return _mm_xor_ps(a, _mm_and_ps(mask, _mm_xor_ps(b, a)));
#endif
}

// Logical operations =============================================================================

Vector4 Vector4::operator& (const Vector4& b) const
{
    return _mm_and_ps(v, b);
}

Vector4 Vector4::operator| (const Vector4& b) const
{
    return _mm_or_ps(v, b);
}

Vector4 Vector4::operator^ (const Vector4& b) const
{
    return _mm_xor_ps(v, b);
}

Vector4& Vector4::operator&= (const Vector4& b)
{
    v = _mm_and_ps(v, b);
    return *this;
}

Vector4& Vector4::operator|= (const Vector4& b)
{
    v = _mm_or_ps(v, b);
    return *this;
}

Vector4& Vector4::operator^= (const Vector4& b)
{
    v = _mm_xor_ps(v, b);
    return *this;
}

// Simple arithmetics =============================================================================

Vector4 Vector4::operator- () const
{
    return Vector4() - (*this);
}

Vector4 Vector4::operator+ (const Vector4& b) const
{
    return _mm_add_ps(v, b);
}

Vector4 Vector4::operator- (const Vector4& b) const
{
    return _mm_sub_ps(v, b);
}

Vector4 Vector4::operator* (const Vector4& b) const
{
    return _mm_mul_ps(v, b);
}

Vector4 Vector4::operator/ (const Vector4& b) const
{
    return _mm_div_ps(v, b);
}

Vector4 Vector4::operator* (float b) const
{
    return _mm_mul_ps(v, _mm_set1_ps(b));
}

Vector4 Vector4::operator/ (float b) const
{
    return _mm_div_ps(v, _mm_set1_ps(b));
}

Vector4 operator*(float a, const Vector4& b)
{
    return _mm_mul_ps(b, _mm_set1_ps(a));
}


Vector4& Vector4::operator+= (const Vector4& b)
{
    v = _mm_add_ps(v, b);
    return *this;
}

Vector4& Vector4::operator-= (const Vector4& b)
{
    v = _mm_sub_ps(v, b);
    return *this;
}

Vector4& Vector4::operator*= (const Vector4& b)
{
    v = _mm_mul_ps(v, b);
    return *this;
}

Vector4& Vector4::operator/= (const Vector4& b)
{
    v = _mm_div_ps(v, b);
    return *this;
}

Vector4& Vector4::operator*= (float b)
{
    v = _mm_mul_ps(v, _mm_set1_ps(b));
    return *this;
}

Vector4& Vector4::operator/= (float b)
{
    v = _mm_div_ps(v, _mm_set1_ps(b));
    return *this;
}

Vector4 Vector4::MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef NFE_USE_FMA
    return _mm_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

Vector4 Vector4::MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef NFE_USE_FMA
    return _mm_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

Vector4 Vector4::NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef NFE_USE_FMA
    return _mm_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

Vector4 Vector4::NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef NFE_USE_FMA
    return _mm_fnmsub_ps(a, b, c);
#else
    return -(a * b) - c;
#endif
}

Vector4 Vector4::Floor(const Vector4& V)
{
    Vector4 vResult = _mm_sub_ps(V, _mm_set1_ps(0.49999f));
    __m128i vInt = _mm_cvtps_epi32(vResult);
    vResult = _mm_cvtepi32_ps(vInt);
    return vResult;
}

Vector4 Vector4::Sqrt(const Vector4& V)
{
    return _mm_sqrt_ss(V);
}

Vector4 Vector4::Sqrt4(const Vector4& V)
{
    return _mm_sqrt_ps(V);
}

Vector4 Vector4::Reciprocal(const Vector4& V)
{
    return _mm_div_ps(VECTOR_ONE, V);
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight)
{
    __m128 vTemp = _mm_sub_ps(v2, v1);
    vTemp = _mm_mul_ps(vTemp, weight);
    return _mm_add_ps(v1, vTemp);
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, float weight)
{
    __m128 vWeight = _mm_set_ps1(weight);
    __m128 vTemp = _mm_sub_ps(v2, v1);
    vTemp = _mm_mul_ps(vTemp, vWeight);
    return _mm_add_ps(v1, vTemp);
}

Vector4 Vector4::Min(const Vector4& a, const Vector4& b)
{
    return _mm_min_ps(a, b);
}

Vector4 Vector4::Max(const Vector4& a, const Vector4& b)
{
    return _mm_max_ps(a, b);
}

Vector4 Vector4::Abs(const Vector4& v)
{
    return _mm_and_ps(v, VECTOR_MASK_ABS);
}

// Comparison functions ===========================================================================

int Vector4::EqualMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpeq_ps(v1, v2));
}

int Vector4::LessMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmplt_ps(v1, v2));
}

int Vector4::LessEqMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmple_ps(v1, v2));
}

int Vector4::GreaterMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpgt_ps(v1, v2));
}

int Vector4::GreaterEqMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpge_ps(v1, v2));
}

int Vector4::NotEqualMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpneq_ps(v1, v2));
}

// 2D vector comparison functions =================================================================

bool Vector4::Equal2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpeq_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::Less2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmplt_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::LessEq2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmple_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::Greater2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpgt_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::GreaterEq2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpge_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::NotEqual2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpneq_ps(v1, v2)) & 0x3) == 0x3;
}

// 3D vector comparison functions =================================================================

bool Vector4::Equal3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpeq_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::Less3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmplt_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::LessEq3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmple_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::Greater3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpgt_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::GreaterEq3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpge_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::NotEqual3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpneq_ps(v1, v2)) & 0x7) == 0x7;
}

// 4D vector comparison functions =================================================================

bool Vector4::operator== (const Vector4& b) const
{
    return EqualMask(*this, b) == 0xF;
}

bool Vector4::operator< (const Vector4& b) const
{
    return LessMask(*this, b) == 0xF;
}

bool Vector4::operator<= (const Vector4& b) const
{
    return LessEqMask(*this, b) == 0xF;
}

bool Vector4::operator> (const Vector4& b) const
{
    return GreaterMask(*this, b) == 0xF;
}

bool Vector4::operator>= (const Vector4& b) const
{
    return GreaterEqMask(*this, b) == 0xF;
}

bool Vector4::operator!= (const Vector4& b) const
{
    return NotEqualMask(*this, b) == 0xF;
}

// Geometry functions =============================================================================

Vector4 Vector4::Dot2V(const Vector4& v1, const Vector4& v2)
{
#ifdef NFE_USE_SSE4
    return _mm_dp_ps(v1, v2, 0x3F);
#else
    __m128 vDot = _mm_mul_ps(v1, v2);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    return _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));
#endif // NFE_USE_SSE4
}

float Vector4::Dot2(const Vector4& v1, const Vector4& v2)
{
    float result;
    _mm_store_ss(&result, Dot2V(v1, v2));
    return result;
}

Vector4 Vector4::Dot3V(const Vector4& v1, const Vector4& v2)
{
#ifdef NFE_USE_SSE4
    return _mm_dp_ps(v1, v2, 0x7F);
#else
    __m128 vDot = _mm_mul_ps(v1, v2);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    return _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));
#endif // NFE_USE_SSE4
}

float Vector4::Dot3(const Vector4& v1, const Vector4& v2)
{
    float result;
    _mm_store_ss(&result, Dot3V(v1, v2));
    return result;
}

Vector4 Vector4::Dot4V(const Vector4& v1, const Vector4& v2)
{
#ifdef NFE_USE_SSE4
    return _mm_dp_ps(v1, v2, 0xFF);
#else
    __m128 vTemp = _mm_mul_ps(v1, v2);
    __m128 vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 3, 2));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_add_ss(vTemp, vTemp2);
    return _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
#endif // NFE_USE_SSE4
}

float Vector4::Dot4(const Vector4& v1, const Vector4& v2)
{
    float result;
    _mm_store_ss(&result, Dot4V(v1, v2));
    return result;
}

Vector4 Vector4::Cross3(const Vector4& V1, const Vector4& V2)
{
    __m128 vTemp1 = _mm_shuffle_ps(V1, V1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 vTemp2 = _mm_shuffle_ps(V2, V2, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
    vTemp1 = _mm_shuffle_ps(vTemp1, vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
    vResult = NegMulAndAdd(vTemp1, vTemp2, vResult);
    return _mm_and_ps(vResult, VECTOR_MASK_XYZ);
}

float Vector4::Length2() const
{
    float result;
    _mm_store_ss(&result, Length2V());
    return result;
}

Vector4 Vector4::Length2V() const
{
    return Sqrt4(Dot2V(v, v));
}

float Vector4::Length3() const
{
    float result;
    _mm_store_ss(&result, Length3V());
    return result;
}

Vector4 Vector4::Length3V() const
{
    return Sqrt4(Dot3V(v, v));
}

float Vector4::Length4() const
{
    float result;
    _mm_store_ss(&result, Length4V());
    return result;
}

Vector4 Vector4::Length4V() const
{
    return Sqrt4(Dot4V(v, v));
}

Vector4& Vector4::Normalize2()
{
    *this /= Length2V();
    return *this;
}

Vector4 Vector4::Normalized2() const
{
    Vector4 result = *this;
    result.Normalize2();
    return result;
}

Vector4& Vector4::Normalize3()
{
    *this /= Length3V();
    return *this;
}

Vector4 Vector4::Normalized3() const
{
    Vector4 result = *this;
    result.Normalize3();
    return result;
}

Vector4& Vector4::Normalize4()
{
    *this /= Length4V();
    return *this;
}

Vector4 Vector4::Normalized4() const
{
    Vector4 result = *this;
    result.Normalize4();
    return result;
}

Vector4 Vector4::Reflect3(const Vector4& i, const Vector4& n)
{
    const __m128 vDot = Dot3V(i, n);
    __m128 vTemp = _mm_add_ps(vDot, vDot); // vTemp = 2 * vDot
    vTemp = _mm_mul_ps(vTemp, n);
    return _mm_sub_ps(i, vTemp);
}

} // namespace Math
} // namespace NFE
