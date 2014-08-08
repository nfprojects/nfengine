/**
 * @file   Vector.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Vector class declaration and definitions of inline functions.
 */

#pragma once

namespace NFE {
namespace Math {

/**
 * 4 element vector of floats.
 * @details Used to perform operations on 4D, 3D and 2D vectors. The content is mapped to
 *          SSE registers to speedup computations.
 */
NFE_ALIGN(16)
struct Vector
{
    union
    {
        unsigned int i[4];
        float f[4];
        __m128 v;
    };

    /// conversion to SIMD types
    NFE_INLINE operator __m128() const
    {
        return v;
    }
    NFE_INLINE operator __m128i() const
    {
        return reinterpret_cast<const __m128i*>(&v)[0];
    }
    NFE_INLINE operator __m128d() const
    {
        return reinterpret_cast<const __m128d*>(&v)[0];
    }

    /// constructors
    NFE_INLINE Vector();
    NFE_INLINE Vector(const __m128& src);
    NFE_INLINE Vector(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    NFE_INLINE Vector(const float* pSrc);
    NFE_INLINE Vector(const Float2& src);
    NFE_INLINE Vector(const Float3& src);
    NFE_INLINE Vector(const Float4& src);

    NFE_INLINE void Set(float f_)
    {
        v = _mm_set1_ps(f_);
    }

    // element access
    float& operator[] (int index)
    {
        return f[index];
    }

    /// simple arithmetics
    NFE_INLINE Vector operator- () const;
    NFE_INLINE Vector operator+ (const Vector& b) const;
    NFE_INLINE Vector operator- (const Vector& b) const;
    NFE_INLINE Vector operator* (const Vector& b) const;
    NFE_INLINE Vector operator/ (const Vector& b) const;
    NFE_INLINE Vector operator* (float b) const;
    NFE_INLINE Vector operator/ (float b) const;
    NFE_INLINE Vector& operator+= (const Vector& b);
    NFE_INLINE Vector& operator-= (const Vector& b);
    NFE_INLINE Vector& operator*= (const Vector& b);
    NFE_INLINE Vector& operator/= (const Vector& b);
    NFE_INLINE Vector& operator*= (float b);
    NFE_INLINE Vector& operator/= (float b);

    /// comparison operators (returns ture, if all the elements satisfy the equation)
    NFE_INLINE bool operator== (const Vector& b) const;
    NFE_INLINE bool operator< (const Vector& b) const;
    NFE_INLINE bool operator<= (const Vector& b) const;
    NFE_INLINE bool operator> (const Vector& b) const;
    NFE_INLINE bool operator>= (const Vector& b) const;
    NFE_INLINE bool operator!= (const Vector& b) const;

    /// bitwise logic operations
    NFE_INLINE Vector operator& (const Vector& b) const;
    NFE_INLINE Vector operator| (const Vector& b) const;
    NFE_INLINE Vector operator^ (const Vector& b) const;
    NFE_INLINE Vector& operator&= (const Vector& b);
    NFE_INLINE Vector& operator|= (const Vector& b);
    NFE_INLINE Vector& operator^= (const Vector& b);

    NFE_INLINE Vector SplatX() const;
    NFE_INLINE Vector SplatY() const;
    NFE_INLINE Vector SplatZ() const;
    NFE_INLINE Vector SplatW() const;
};

// used to initialize float constants
NFE_ALIGN(16)
struct Vectorf
{
    union
    {
        float f[4];
        __m128 v;
    };

    NFE_INLINE operator Vector() const
    {
        Vector temp;
        temp.v = v;
        return temp;
    }

    NFE_INLINE operator __m128() const
    {
        return v;
    }
    NFE_INLINE operator __m128i() const
    {
        return reinterpret_cast<const __m128i*>(&v)[0];
    }
    NFE_INLINE operator __m128d() const
    {
        return reinterpret_cast<const __m128d*>(&v)[0];
    }
};

// used to initialize integer constants
NFE_ALIGN(16)
struct Vectori
{
    union
    {
        unsigned int u[4];
        __m128 v;
    };

    NFE_INLINE operator Vector() const
    {
        Vector temp;
        temp.v = v;
        return temp;
    }

    NFE_INLINE operator __m128() const
    {
        return v;
    }
    NFE_INLINE operator __m128i() const
    {
        return reinterpret_cast<const __m128i*>(&v)[0];
    }
    NFE_INLINE operator __m128d() const
    {
        return reinterpret_cast<const __m128d*>(&v)[0];
    }
};


//
// Constants definitions
//
const Vectorf g_Epsilon = {NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON};
const Vectorf g_One = {1.0f, 1.0f, 1.0f, 1.0f};
const Vectorf g_One3 = {1.0f, 1.0f, 1.0f, 0.0f};
const Vectorf g_MinusOne = { -1.0f, -1.0f, -1.0f, -1.0f};
const Vectori g_MaskX = {0xFFFFFFFF, 0, 0, 0};
const Vectori g_MaskY = {0, 0xFFFFFFFF, 0, 0};
const Vectori g_MaskZ = {0, 0, 0xFFFFFFFF, 0};
const Vectori g_MaskW = {0, 0, 0, 0xFFFFFFFF};
const Vectori g_Mask3 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0};
const Vectori g_AbsMask = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

const Vectorf g_Byte2Float = {1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f};
const Vectorf g_Float2Byte = {255.0f, 255.0f, 255.0f, 255.0f};


// identity matrix rows
const Vectorf g_IdentityR0  = {1.0f, 0.0f, 0.0f, 0.0f};
const Vectorf g_IdentityR1  = {0.0f, 1.0f, 0.0f, 0.0f};
const Vectorf g_IdentityR2  = {0.0f, 0.0f, 1.0f, 0.0f};
const Vectorf g_IdentityR3  = {0.0f, 0.0f, 0.0f, 1.0f};

// ====================================================================================

Vector::Vector()
{
    v = _mm_setzero_ps();
}

Vector::Vector(float x, float y, float z, float w)
{
    v = _mm_set_ps(w, z, y, x);
}

// copy array of 4 floats
Vector::Vector(const float* pSrc)
{
    v = _mm_loadu_ps(pSrc);
}

Vector::Vector(const __m128& src)
{
    v = src;
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


/**
 * Convert 4 unsigned chars to a Vector.
 */
NFE_INLINE Vector VectorLoadUChar4(const unsigned char* pSrc)
{
    static const Vectori mask = {0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    static const __m128 LoadUByte4Mul = {1.0f, 1.0f / 256.0f, 1.0f / 65536.0f, 1.0f / (65536.0f * 256.0f)};
    static const Vectori flipW = {0, 0, 0, 0x80000000};
    static const __m128 unsignedOffset = {0, 0, 0, 32768.0f * 65536.0f};

    __m128 vTemp = _mm_load_ps1((const float*)pSrc);
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
NFE_INLINE void VectorStoreUChar4(const Vector& src, unsigned char* pDest)
{
    static const __m128 MaxUByte4 = { 255.0f, 255.0f, 255.0f, 255.0f};

    // Clamp
    __m128 vResult = _mm_max_ps(src, _mm_setzero_ps());
    vResult = _mm_min_ps(vResult, MaxUByte4);

    // Convert to int & extract components
    __m128i vResulti = _mm_cvttps_epi32(vResult);
    __m128i Yi = _mm_srli_si128(vResulti, 3);
    __m128i Zi = _mm_srli_si128(vResulti, 6);
    __m128i Wi = _mm_srli_si128(vResulti, 9);

    vResulti = _mm_or_si128(_mm_or_si128(Wi, Zi), _mm_or_si128(Yi, vResulti));
    _mm_store_ss(reinterpret_cast<float*>(pDest), reinterpret_cast<const __m128*>(&vResulti)[0]);
}



NFE_INLINE void VectorStore(const Vector& src, float* pDest)
{
    _mm_store_ss(pDest, src);
}

NFE_INLINE void VectorStore(const Vector& src, Float2* pDest)
{
    __m128 y = _mm_shuffle_ps(src, src, _MM_SHUFFLE(1, 1, 1, 1));
    _mm_store_ss(&pDest->x, src);
    _mm_store_ss(&pDest->y, y);
}

NFE_INLINE void VectorStore(const Vector& src, Float3* pDest)
{
    __m128 y = _mm_shuffle_ps(src, src, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 z = _mm_shuffle_ps(src, src, _MM_SHUFFLE(2, 2, 2, 2));
    _mm_store_ss(&pDest->x, src);
    _mm_store_ss(&pDest->y, y);
    _mm_store_ss(&pDest->z, z);
}

NFE_INLINE void VectorStore(const Vector& src, Float4* pDest)
{
    _mm_storeu_ps(&pDest->x, src);
}

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

NFE_INLINE Vector operator*(float a, const Vector& b)
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


NFE_INLINE Vector VectorFloor(const Vector& V)
{
    Vector vResult = _mm_sub_ps(V, _mm_set1_ps(0.49999f));
    __m128i vInt = _mm_cvtps_epi32(vResult);
    vResult = _mm_cvtepi32_ps(vInt);
    return vResult;
}


NFE_INLINE Vector VectorSqrt(const Vector& V)
{
    return _mm_sqrt_ss(V);
}


NFE_INLINE Vector VectorSqrt4(const Vector& V)
{
    return _mm_sqrt_ps(V);
}

NFE_INLINE Vector VectorReciprocal(const Vector& V)
{
    return _mm_div_ps(g_One, V);
}

NFE_INLINE Vector VectorLerp(const Vector& v1, const Vector& v2, const Vector& weight)
{
    __m128 vTemp = _mm_sub_ps(v2, v1);
    vTemp = _mm_mul_ps(vTemp, weight);
    return _mm_add_ps(v1, vTemp);
}

NFE_INLINE Vector VectorLerp(const Vector& v1, const Vector& v2, float weight)
{
    __m128 vWeight = _mm_set_ps1(weight);
    __m128 vTemp = _mm_sub_ps(v2, v1);
    vTemp = _mm_mul_ps(vTemp, vWeight);
    return _mm_add_ps(v1, vTemp);
}


NFE_INLINE Vector Vector::SplatX() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}

NFE_INLINE Vector Vector::SplatY() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}

NFE_INLINE Vector Vector::SplatZ() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}

NFE_INLINE Vector Vector::SplatW() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

NFE_INLINE Vector VectorSplat(float f)
{
    return _mm_set_ps1(f);
}

NFE_INLINE Vector VectorMin(const Vector& a, const Vector& b)
{
    return _mm_min_ps(a, b);
}

NFE_INLINE Vector VectorMax(const Vector& a, const Vector& b)
{
    return _mm_max_ps(a, b);
}

NFE_INLINE Vector VectorAbs(const Vector& v)
{
    return _mm_and_ps(v, g_AbsMask);
}

// Comparison functions ===========================================================================

NFE_INLINE int VectorEqualMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpeq_ps(v1, v2));
}

NFE_INLINE int VectorLessMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmplt_ps(v1, v2));
}

NFE_INLINE int VectorLessEqMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmple_ps(v1, v2));
}

NFE_INLINE int VectorGreaterMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpgt_ps(v1, v2));
}

NFE_INLINE int VectorGreaterEqMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpge_ps(v1, v2));
}

NFE_INLINE int VectorNotEqualMask(const Vector& v1, const Vector& v2)
{
    return _mm_movemask_ps(_mm_cmpneq_ps(v1, v2));
}

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


/**
 * Calculate 3D dot product.
 * @return Vector of dot products.
 */
NFE_INLINE Vector VectorDot3(const Vector& v1, const Vector& v2)
{
    __m128 vDot = _mm_mul_ps(v1, v2);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    return _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));
}

/**
 * Calculate 3D dot product.
 * @return Vector of dot products.
 */
NFE_INLINE float VectorDot3f(const Vector& v1, const Vector& v2)
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

/**
 * Calculate 3D cross product.
 * @return Vector of dot products.
 */
NFE_INLINE Vector VectorCross3(const Vector& V1, const Vector& V2)
{
    __m128 vTemp1 = _mm_shuffle_ps(V1, V1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 vTemp2 = _mm_shuffle_ps(V2, V2, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
    vTemp1 = _mm_shuffle_ps(vTemp1, vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
    vTemp1 = _mm_mul_ps(vTemp1, vTemp2);
    vResult = _mm_sub_ps(vResult, vTemp1);
    return _mm_and_ps(vResult, g_Mask3);
}

/**
 * Calculate length of a 3D vector.
 * @details 4th element is ignored.
 * @return Vector of @p v length.
 */
NFE_INLINE Vector VectorLength3(const Vector& v)
{
    __m128 vDot = _mm_mul_ps(v, v);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vDot = _mm_sqrt_ss(vDot);
    return _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));
}

/**
 * Normalize 3D vector.
 * @details 4th element is ignored.
 */
NFE_INLINE Vector VectorNormalize3(const Vector& v)
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

/**
 * Reflect a 3D vector.
 * @param I incident vector
 * @param N normal vector
 * @details 4th element is ignored.
 */
NFE_INLINE Vector VectorReflect3(const Vector& I, const Vector& N)
{
    __m128 vDot = _mm_mul_ps(I, N);
    __m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vDot = _mm_add_ss(vDot, vTemp);
    vDot = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));

    vTemp = _mm_add_ps(vDot, vDot); // vTemp = 2 * vDot
    vTemp = _mm_mul_ps(vTemp, N);
    return _mm_sub_ps(I, vTemp);
}

/**
 * Calculate 4D dot product.
 * @return Vector of dot products.
 */
NFE_INLINE Vector VectorDot4(const Vector& v1, const Vector& v2)
{
    __m128 vTemp = _mm_mul_ps(v1, v2);
    __m128 vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 3, 2));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_add_ss(vTemp, vTemp2);
    return _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
}

/**
 * Calculate length of a 4D vector.
 * @return Vector of @p v length.
 */
NFE_INLINE Vector VectorLength4(const Vector& v)
{
    __m128 vTemp = _mm_mul_ps(v, v);
    __m128 vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 3, 2));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_add_ss(vTemp, vTemp2);
    return _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
    return _mm_sqrt_ps(vTemp);
}

/**
 * Normalize 4D vector.
 */
NFE_INLINE Vector VectorNormalize4(const Vector& v)
{
    __m128 vTemp = _mm_mul_ps(v, v);
    __m128 vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 3, 2));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_add_ss(vTemp, vTemp2);
    vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(0, 0, 0, 0));
    return _mm_div_ps(v, _mm_sqrt_ps(vTemp));
}

/**
 * Generate a plane equation from 3 points.
 * @param p1,p2,p3 Planar points
 * @return Plane equation
 */
NFE_INLINE Vector PlaneFromPoints(const Vector& p1, const Vector& p2, const Vector& p3)
{
    Vector N;
    Vector D;
    Vector V21;
    Vector V31;
    Vector Result;

    V21 = _mm_sub_ps(p1, p2);
    V31 = _mm_sub_ps(p1, p3);

    N = VectorCross3(V21, V31);
    N = VectorNormalize3(N);

    D = VectorDot3(N, p1);
    D = _mm_mul_ps(D, g_MinusOne);
    N = _mm_and_ps(N, g_Mask3);
    D = _mm_and_ps(D, g_MaskW);
    Result = _mm_or_ps(D, N);
    return Result;
}

/**
 * Calculates dot product of a plane and a point.
 * @return Vector of results
 */
NFE_INLINE Vector PlanePointDot3(const Vector& plane, const Vector& point)
{
    /*
            Vector vTemp2 = _mm_shuffle_ps(Plane, Plane, _MM_SHUFFLE(3,3,3,3));
            Vector vTemp = _mm_dp_ps(Plane, Point, 0x7F);
            return _mm_add_ps(vTemp2, vTemp);
    */
    Vector vTemp2 = _mm_and_ps(point, g_Mask3);
    vTemp2 = _mm_or_ps(vTemp2, g_IdentityR3);
    Vector vTemp = _mm_mul_ps(plane, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0,
                            0)); // Copy X to the Z position and Y to the W position
    vTemp2 = _mm_add_ps(vTemp2, vTemp);         // Add Z = X+Z; W = Y+W;
    vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0)); // Copy W to the Z position
    vTemp = _mm_add_ps(vTemp, vTemp2);           // Add Z and W together
    return _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(2, 2, 2, 2)); // Splat Z and return
}

} // namespace Math
} // namespace NFE
