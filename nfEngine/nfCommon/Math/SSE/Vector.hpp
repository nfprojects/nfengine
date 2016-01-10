/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  SSE version of Vector class definitions.
 */

#pragma once

#if !defined(NFE_USE_SSE)
#error "NFE_USE_SSE must be defined when including this header"
#endif

namespace NFE {
namespace Math {

// Constructors ===================================================================================

Vector::Vector()
{
    v = _mm_setzero_ps();
}

Vector::Vector(float x, float y, float z, float w)
{
    v = _mm_set_ps(w, z, y, x);
}

Vector::Vector(int x, int y, int z, int w)
{
    v = _mm_castsi128_ps(_mm_set_epi32(w, z, y, x));
}

// copy array of 4 floats
Vector::Vector(const float* src)
{
    v = _mm_loadu_ps(src);
}

Vector::Vector(const Float2& src)
{
    __m128 x = _mm_load_ss(&src.x);
    __m128 y = _mm_load_ss(&src.y);
    v = _mm_unpacklo_ps(x, y);
}

Vector::Vector(const Float3& src)
{
    __m128 x = _mm_load_ss(&src.x);
    __m128 y = _mm_load_ss(&src.y);
    __m128 z = _mm_load_ss(&src.z);
    __m128 xy = _mm_unpacklo_ps(x, y);
    v = _mm_movelh_ps(xy, z);
}

Vector::Vector(const Float4& src)
{
    v = _mm_loadu_ps(&src.x);
}

void Vector::Set(float scalar)
{
    v = _mm_set1_ps(scalar);
}

// Load & store ===================================================================================

Vector VectorLoadUChar4(const unsigned char* src)
{
    static const Vectori mask = {{{0xFF, 0xFF00, 0xFF0000, 0xFF000000}}};
    static const __m128 LoadUByte4Mul = {1.0f, 1.0f / 256.0f, 1.0f / 65536.0f,
                                         1.0f / (65536.0f * 256.0f)};
    static const Vectori flipW = {{{0, 0, 0, 0x80000000}}};
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
 * Convert a Vector to 4 unsigned chars.
 */
void VectorStoreUChar4(const Vector& src, unsigned char* dest)
{
    static const __m128 MaxUByte4 = {255.0f, 255.0f, 255.0f, 255.0f};

    // Clamp
    __m128 vResult = _mm_max_ps(src, _mm_setzero_ps());
    vResult = _mm_min_ps(vResult, MaxUByte4);

    // Convert to int & extract components
    __m128i vResulti = _mm_cvttps_epi32(vResult);
    __m128i Yi = _mm_srli_si128(vResulti, 3);
    __m128i Zi = _mm_srli_si128(vResulti, 6);
    __m128i Wi = _mm_srli_si128(vResulti, 9);

    vResulti = _mm_or_si128(_mm_or_si128(Wi, Zi), _mm_or_si128(Yi, vResulti));
    _mm_store_ss(reinterpret_cast<float*>(dest), reinterpret_cast<const __m128*>(&vResulti)[0]);
}



void VectorStore(const Vector& src, float* dest)
{
    _mm_store_ss(dest, src);
}

void VectorStore(const Vector& src, Float2* dest)
{
    __m128 y = _mm_shuffle_ps(src, src, _MM_SHUFFLE(1, 1, 1, 1));
    _mm_store_ss(&dest->x, src);
    _mm_store_ss(&dest->y, y);
}

void VectorStore(const Vector& src, Float3* dest)
{
    __m128 y = _mm_shuffle_ps(src, src, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 z = _mm_shuffle_ps(src, src, _MM_SHUFFLE(2, 2, 2, 2));
    _mm_store_ss(&dest->x, src);
    _mm_store_ss(&dest->y, y);
    _mm_store_ss(&dest->z, z);
}

void VectorStore(const Vector& src, Float4* dest)
{
    _mm_storeu_ps(&dest->x, src);
}

Vector Vector::SplatX() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}

Vector Vector::SplatY() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}

Vector Vector::SplatZ() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}

Vector Vector::SplatW() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

Vector VectorSplat(float f)
{
    return _mm_set_ps1(f);
}

Vector VectorSelectBySign(const Vector& a, const Vector& b, const Vector& sel)
{
#if defined(NFE_USE_SSE4)
    return _mm_blendv_ps(a, b, sel);
#else
    Vector mask = _mm_cmplt_ps(sel, _mm_setzero_ps());
    return _mm_xor_ps(a, _mm_and_ps(mask, _mm_xor_ps(b, a)));
#endif
}

// Logical operations =============================================================================

Vector Vector::operator& (const Vector& b) const
{
    return _mm_and_ps(v, b);
}

Vector Vector::operator| (const Vector& b) const
{
    return _mm_or_ps(v, b);
}

Vector Vector::operator^ (const Vector& b) const
{
    return _mm_xor_ps(v, b);
}

Vector& Vector::operator&= (const Vector& b)
{
    v = _mm_and_ps(v, b);
    return *this;
}

Vector& Vector::operator|= (const Vector& b)
{
    v = _mm_or_ps(v, b);
    return *this;
}

Vector& Vector::operator^= (const Vector& b)
{
    v = _mm_xor_ps(v, b);
    return *this;
}

// Simple arithmetics =============================================================================

Vector Vector::operator- () const
{
    return Vector() - (*this);
}

Vector Vector::operator+ (const Vector& b) const
{
    return _mm_add_ps(v, b);
}

Vector Vector::operator- (const Vector& b) const
{
    return _mm_sub_ps(v, b);
}

Vector Vector::operator* (const Vector& b) const
{
    return _mm_mul_ps(v, b);
}

Vector Vector::operator/ (const Vector& b) const
{
    return _mm_div_ps(v, b);
}

Vector Vector::operator* (float b) const
{
    return _mm_mul_ps(v, _mm_set1_ps(b));
}

Vector Vector::operator/ (float b) const
{
    return _mm_div_ps(v, _mm_set1_ps(b));
}

Vector operator*(float a, const Vector& b)
{
    return _mm_mul_ps(b, _mm_set1_ps(a));
}


Vector& Vector::operator+= (const Vector& b)
{
    v = _mm_add_ps(v, b);
    return *this;
}

Vector& Vector::operator-= (const Vector& b)
{
    v = _mm_sub_ps(v, b);
    return *this;
}

Vector& Vector::operator*= (const Vector& b)
{
    v = _mm_mul_ps(v, b);
    return *this;
}

Vector& Vector::operator/= (const Vector& b)
{
    v = _mm_div_ps(v, b);
    return *this;
}

Vector& Vector::operator*= (float b)
{
    v = _mm_mul_ps(v, _mm_set1_ps(b));
    return *this;
}

Vector& Vector::operator/= (float b)
{
    v = _mm_div_ps(v, _mm_set1_ps(b));
    return *this;
}


Vector VectorFloor(const Vector& V)
{
    Vector vResult = _mm_sub_ps(V, _mm_set1_ps(0.49999f));
    __m128i vInt = _mm_cvtps_epi32(vResult);
    vResult = _mm_cvtepi32_ps(vInt);
    return vResult;
}


Vector VectorSqrt(const Vector& V)
{
    return _mm_sqrt_ss(V);
}


Vector VectorSqrt4(const Vector& V)
{
    return _mm_sqrt_ps(V);
}

Vector VectorReciprocal(const Vector& V)
{
    return _mm_div_ps(VECTOR_ONE, V);
}

Vector VectorLerp(const Vector& v1, const Vector& v2, const Vector& weight)
{
    __m128 vTemp = _mm_sub_ps(v2, v1);
    vTemp = _mm_mul_ps(vTemp, weight);
    return _mm_add_ps(v1, vTemp);
}

Vector VectorLerp(const Vector& v1, const Vector& v2, float weight)
{
    __m128 vWeight = _mm_set_ps1(weight);
    __m128 vTemp = _mm_sub_ps(v2, v1);
    vTemp = _mm_mul_ps(vTemp, vWeight);
    return _mm_add_ps(v1, vTemp);
}

Vector VectorMin(const Vector& a, const Vector& b)
{
    return _mm_min_ps(a, b);
}

Vector VectorMax(const Vector& a, const Vector& b)
{
    return _mm_max_ps(a, b);
}

Vector VectorAbs(const Vector& v)
{
    return _mm_and_ps(v, VECTOR_MASK_ABS);
}

// Comparison functions ===========================================================================

int VectorEqualMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpeq_ps(v1, v2));
}

int VectorLessMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmplt_ps(v1, v2));
}

int VectorLessEqMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmple_ps(v1, v2));
}

int VectorGreaterMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpgt_ps(v1, v2));
}

int VectorGreaterEqMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpge_ps(v1, v2));
}

int VectorNotEqualMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpneq_ps(v1, v2));
}

// 2D vector comparison functions =================================================================

bool VectorEqual2(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpeq_ps(v1, v2)) & 0x3) == 0x3;
}

bool VectorLess2(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmplt_ps(v1, v2)) & 0x3) == 0x3;
}

bool VectorLessEq2(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmple_ps(v1, v2)) & 0x3) == 0x3;
}

bool VectorGreater2(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpgt_ps(v1, v2)) & 0x3) == 0x3;
}

bool VectorGreaterEq2(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpge_ps(v1, v2)) & 0x3) == 0x3;
}

bool VectorNotEqual2(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpneq_ps(v1, v2)) & 0x3) == 0x3;
}

// 3D vector comparison functions =================================================================

bool VectorEqual3(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpeq_ps(v1, v2)) & 0x7) == 0x7;
}

bool VectorLess3(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmplt_ps(v1, v2)) & 0x7) == 0x7;
}

bool VectorLessEq3(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmple_ps(v1, v2)) & 0x7) == 0x7;
}

bool VectorGreater3(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpgt_ps(v1, v2)) & 0x7) == 0x7;
}

bool VectorGreaterEq3(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpge_ps(v1, v2)) & 0x7) == 0x7;
}

bool VectorNotEqual3(const Vector& v1, const Vector& v2)
{
    return (_mm_movemask_ps(_mm_cmpneq_ps(v1, v2)) & 0x7) == 0x7;
}

// 4D vector comparison functions =================================================================

bool Vector::operator== (const Vector& b) const
{
    return VectorEqualMask(*this, b) == 0xF;
}

bool Vector::operator< (const Vector& b) const
{
    return VectorLessMask(*this, b) == 0xF;
}

bool Vector::operator<= (const Vector& b) const
{
    return VectorLessEqMask(*this, b) == 0xF;
}

bool Vector::operator> (const Vector& b) const
{
    return VectorGreaterMask(*this, b) == 0xF;
}

bool Vector::operator>= (const Vector& b) const
{
    return VectorGreaterEqMask(*this, b) == 0xF;
}

bool Vector::operator!= (const Vector& b) const
{
    return VectorNotEqualMask(*this, b) == 0xF;
}

// Geometry functions =============================================================================

Vector VectorDot3(const Vector& v1, const Vector& v2)
{
    __m128 vDot = _mm_mul_ps(v1, v2);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    return _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));
}

float VectorDot3f(const Vector& v1, const Vector& v2)
{
    float result;

    __m128 vDot = _mm_mul_ps(v1, v2);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    _mm_store_ss(&result, vDot);

    return result;
}

Vector VectorCross3(const Vector& V1, const Vector& V2)
{
    __m128 vTemp1 = _mm_shuffle_ps(V1, V1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 vTemp2 = _mm_shuffle_ps(V2, V2, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
    vTemp1 = _mm_shuffle_ps(vTemp1, vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
    vTemp1 = _mm_mul_ps(vTemp1, vTemp2);
    vResult = _mm_sub_ps(vResult, vTemp1);
    return _mm_and_ps(vResult, VECTOR_MASK_XYZ);
}

Vector VectorLength3(const Vector& v)
{
    __m128 vDot = _mm_mul_ps(v, v);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vDot = _mm_sqrt_ss(vDot);
    return _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));
}

Vector VectorNormalize3(const Vector& v)
{
    __m128 vDot = _mm_mul_ps(v, v);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_sqrt_ss(vDot);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
    return _mm_div_ps(v, vTemp);
}

Vector VectorDot4(const Vector& v1, const Vector& v2)
{
    __m128 vTemp = _mm_mul_ps(v1, v2);
    __m128 vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 3, 2));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_add_ss(vTemp, vTemp2);
    return _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
}

Vector VectorLength4(const Vector& v)
{
    __m128 vTemp = _mm_mul_ps(v, v);
    __m128 vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 3, 2));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_add_ss(vTemp, vTemp2);
    return _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
    return _mm_sqrt_ps(vTemp);
}

Vector VectorNormalize4(const Vector& v)
{
    __m128 vTemp = _mm_mul_ps(v, v);
    __m128 vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 3, 2));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_add_ss(vTemp, vTemp2);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
    return _mm_div_ps(v, _mm_sqrt_ps(vTemp));
}

Vector VectorReflect3(const Vector& i, const Vector& n)
{
    __m128 vDot = _mm_mul_ps(i, n);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vDot = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));

    vTemp = _mm_add_ps(vDot, vDot); // vTemp = 2 * vDot
    vTemp = _mm_mul_ps(vTemp, n);
    return _mm_sub_ps(i, vTemp);
}

Vector PlaneFromPoints(const Vector& p1, const Vector& p2, const Vector& p3)
{
    Vector V21 = p1 - p2;
    Vector V31 = p1 - p3;
    Vector n = VectorNormalize3(VectorCross3(V21, V31));
    Vector d = VectorDot3(n, p1);
    d = _mm_mul_ps(d, VECTOR_MINUS_ONE);
    n = _mm_and_ps(n, VECTOR_MASK_XYZ);
    d = _mm_and_ps(d, VECTOR_MASK_W);
    return _mm_or_ps(d, n);
}

Vector PlaneFromNormalAndPoint(const Vector& normal, const Vector& p)
{
    Vector d = VectorDot3(normal, p);
    d = _mm_mul_ps(d, VECTOR_MINUS_ONE);
    Vector n = _mm_and_ps(normal, VECTOR_MASK_XYZ);
    d = _mm_and_ps(d, VECTOR_MASK_W);
    return _mm_or_ps(d, n);
}

bool PlanePointSide(const Vector& plane, const Vector& point)
{
    Vector vTemp2 = _mm_and_ps(point, VECTOR_MASK_XYZ);
    vTemp2 = _mm_or_ps(vTemp2, VECTOR_IDENTITY_ROW_3);
    Vector vTemp = _mm_mul_ps(plane, vTemp2);
    // copy X to the Z position and Y to the W position
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0, 0));
    // add Z = X+Z; W = Y+W
    vTemp2 = _mm_add_ps(vTemp2, vTemp);
    // copy W to the Z position
    vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0));
    // add Z and W together
    vTemp = _mm_add_ps(vTemp, vTemp2);
    // return Z >= 0.0f
    int mask = _mm_movemask_ps(_mm_cmpge_ps(vTemp, _mm_setzero_ps()));
    return (mask & (1 << 2)) != 0;
}

} // namespace Math
} // namespace NFE
