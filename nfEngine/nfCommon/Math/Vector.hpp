/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Vector class declaration and definitions of inline functions.
 */

#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {

/**
 * 4 element vector of floats.
 * @details Used to perform operations on 4D, 3D and 2D vectors. The content is mapped to
 *          SSE registers to speedup computations.
 */
struct NFE_ALIGN16 Vector
{
    union
    {
        float f[4];
        int i[4];
        unsigned int u[4];

#ifdef NFE_USE_SSE
        __m128 v;
#endif
    };

#ifdef NFE_USE_SSE
    /// conversion to/from SSE types
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
    NFE_INLINE Vector(const __m128& src)
    {
        v = src;
    }
#endif

    /// constructors
    NFE_INLINE Vector();
    NFE_INLINE explicit Vector(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    NFE_INLINE explicit Vector(int x, int y = 0, int z = 0, int w = 0);
    NFE_INLINE Vector(const float* src);
    NFE_INLINE Vector(const Float2& src);
    NFE_INLINE Vector(const Float3& src);
    NFE_INLINE Vector(const Float4& src);
    NFE_INLINE void Set(float scalar);

    // element access
    NFE_INLINE float operator[] (int index) const
    {
        return f[index];
    }

    // element access (reference)
    NFE_INLINE float& operator[] (int index)
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

    /// comparison operators (returns true, if all the elements satisfy the equation)
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

    /**
     * Rearrange vector elements.
     */
    template<bool x = false, bool y = false, bool z = false, bool w = false>
    NFE_INLINE Vector ChangeSign() const;

    /**
     * Rearrange vector elements.
     */
    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_INLINE Vector Swizzle() const;

    /**
     * Convert 4 uint8 to a Vector.
     */
    NFE_INLINE static Vector Load4(const uint8* src);

    /**
     * Convert a Vector to 4 unsigned chars.
     */
    NFE_INLINE void Store4(uint8* dest) const;

    NFE_INLINE void Store(float* dest) const;
    NFE_INLINE void Store(Float2* dest) const;
    NFE_INLINE void Store(Float3* dest) const;
    NFE_INLINE void Store(Float4* dest) const;
    NFE_INLINE static Vector Splat(float f);

    NFE_INLINE static Vector Floor(const Vector& v);
    NFE_INLINE static Vector Sqrt(const Vector& v);
    NFE_INLINE static Vector Sqrt4(const Vector& v);
    NFE_INLINE static Vector Reciprocal(const Vector& v);
    NFE_INLINE static Vector Lerp(const Vector& v1, const Vector& v2, const Vector& weight);
    NFE_INLINE static Vector Lerp(const Vector& v1, const Vector& v2, float weight);
    NFE_INLINE static Vector Min(const Vector& a, const Vector& b);
    NFE_INLINE static Vector Max(const Vector& a, const Vector& b);
    NFE_INLINE static Vector Abs(const Vector& v);

    NFE_INLINE static int EqualMask(const Vector& v1, const Vector& v2);
    NFE_INLINE static int LessMask(const Vector& v1, const Vector& v2);
    NFE_INLINE static int LessEqMask(const Vector& v1, const Vector& v2);
    NFE_INLINE static int GreaterMask(const Vector& v1, const Vector& v2);
    NFE_INLINE static int GreaterEqMask(const Vector& v1, const Vector& v2);
    NFE_INLINE static int NotEqualMask(const Vector& v1, const Vector& v2);

    NFE_INLINE static bool Equal2(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool Less2(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool LessEq2(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool Greater2(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool GreaterEq2(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool NotEqual2(const Vector& v1, const Vector& v2);

    NFE_INLINE static bool Equal3(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool Less3(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool LessEq3(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool Greater3(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool GreaterEq3(const Vector& v1, const Vector& v2);
    NFE_INLINE static bool NotEqual3(const Vector& v1, const Vector& v2);

    /**
     * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
     */
    NFE_INLINE static Vector SelectBySign(const Vector& a, const Vector& b, const Vector& sel);

    /**
     * Calculate 3D dot product.
     * @return Dot product (scalar value).
     */
    NFE_INLINE static float Dot3(const Vector& v1, const Vector& v2);

    /**
     * Calculate 3D dot product.
     * @return Vector of dot products.
     */
    NFE_INLINE static Vector Dot3V(const Vector& v1, const Vector& v2);

    /**
     * Calculate 4D dot product.
     * @return Vector of dot products.
     */
    NFE_INLINE static float Dot4(const Vector& v1, const Vector& v2);

    /**
     * Calculate 4D dot product.
     * @return Dot product (scalar value).
     */
    NFE_INLINE static Vector Dot4V(const Vector& v1, const Vector& v2);

    /**
     * Calculate 3D cross product.
     * @return Vector of dot products.
     */
    NFE_INLINE static Vector Cross3(const Vector& v1, const Vector& v2);

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Length of vector @p.
     */
    NFE_INLINE float Length3() const;

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Vector of @p v length.
     */
    NFE_INLINE Vector Length3V() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    NFE_INLINE float Length4() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    NFE_INLINE Vector Length4V() const;

    /**
     * Normalize as 3D vector.
     * @details 4th element is ignored.
     */
    NFE_INLINE Vector& Normalize3();

    /**
     * Normalize as 4D vector.
     */
    NFE_INLINE Vector& Normalize4();

    /**
     * Return normalized 3D vector.
     * @details 4th element is ignored.
     */
    NFE_INLINE Vector Normalized3() const;

    /**
     * Return normalized 4D vector.
     */
    NFE_INLINE Vector Normalized4() const;

    /**
     * Reflect a 3D vector.
     * @param i incident vector
     * @param n normal vector
     * @details 4th element is ignored.
     */
    NFE_INLINE static Vector Reflect3(const Vector& i, const Vector& n);

    /**
     * Generate a plane equation from 3 points.
     * @param p1,p2,p3 Planar points
     * @return Plane equation
     */
    NFE_INLINE static Vector PlaneFromPoints(const Vector& p1, const Vector& p2, const Vector& p3);

    /**
     * Generate a plane equation from a normal and a point.
     * @return Plane equation
     */
    NFE_INLINE static Vector PlaneFromNormalAndPoint(const Vector& normal, const Vector& p);

    /**
     * Determine plane side a point belongs to.
     * @return "true" - positive side, "false" - negative side
     */
    NFE_INLINE static bool PlanePointSide(const Vector& plane, const Vector& point);

    /**
     * Check if two vectors are (almost) equal.
     */
    NFE_INLINE static bool AlmostEqual(const Vector& v1, const Vector& v2, float epsilon = NFE_MATH_EPSILON)
    {
        Vector diff = Abs(v1 - v2);
        Vector epsilonV = Vector::Splat(epsilon);
        return diff < epsilonV;
    }

    /**
     * Fused multiply and add.
     * @return  a * b + c
     */
    NFE_INLINE static Vector MulAndAdd(const Vector& a, const Vector& b, const Vector& c);

    /**
     * Fused multiply and subtract.
     * @return  a * b - c
     */
    NFE_INLINE static Vector MulAndSub(const Vector& a, const Vector& b, const Vector& c);

    /**
     * Fused multiply (negated) and add.
     * @return  - a * b + c
     */
    NFE_INLINE static Vector NegMulAndAdd(const Vector& a, const Vector& b, const Vector& c);

    /**
     * Fused multiply (negated) and subtract.
     * @return  - a * b - c
     */
    NFE_INLINE static Vector NegMulAndSub(const Vector& a, const Vector& b, const Vector& c);
};


// used to initialize float constants
struct NFE_ALIGN16 Vectorf
{
    union
    {
        float f[4];
#ifdef NFE_USE_SSE
        __m128 v;
#endif
    };

#ifdef NFE_USE_SSE
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
#else
    NFE_INLINE operator Vector() const
    {
        Vector temp;
        temp.f[0] = f[0];
        temp.f[1] = f[1];
        temp.f[2] = f[2];
        temp.f[3] = f[3];
        return temp;
    }
#endif // NFE_USE_SSE
};

// used to initialize integer constants
struct NFE_ALIGN16 Vectori
{
#ifdef NFE_USE_SSE
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
#else
    union
    {
        unsigned int u[4];
    };

    NFE_INLINE operator Vector() const
    {
        Vector temp;
        temp.u[0] = u[0];
        temp.u[1] = u[1];
        temp.u[2] = u[2];
        temp.u[3] = u[3];
        return temp;
    }
#endif
};

const Vectorf VECTOR_EPSILON = { { { NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON } } };
const Vectorf VECTOR_ONE = { { { 1.0f, 1.0f, 1.0f, 1.0f } } };
const Vectorf VECTOR_ONE3 = { { { 1.0f, 1.0f, 1.0f, 0.0f } } };
const Vectorf VECTOR_MINUS_ONE = { { { -1.0f, -1.0f, -1.0f, -1.0f } } };
const Vectori VECTOR_MASK_X = { { { 0xFFFFFFFF, 0, 0, 0 } } };
const Vectori VECTOR_MASK_Y = { { { 0, 0xFFFFFFFF, 0, 0 } } };
const Vectori VECTOR_MASK_Z = { { { 0, 0, 0xFFFFFFFF, 0 } } };
const Vectori VECTOR_MASK_W = { { { 0, 0, 0, 0xFFFFFFFF } } };
const Vectori VECTOR_MASK_XYZ = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 } } };
const Vectori VECTOR_MASK_ABS = { { { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF } } };

const Vectorf VECTOR_INV_255 = { { { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f } } };
const Vectorf VECTOR_255 = { { { 255.0f, 255.0f, 255.0f, 255.0f } } };

/// identity matrix rows
const Vectorf VECTOR_X = { { { 1.0f, 0.0f, 0.0f, 0.0f } } };
const Vectorf VECTOR_Y = { { { 0.0f, 1.0f, 0.0f, 0.0f } } };
const Vectorf VECTOR_Z = { { { 0.0f, 0.0f, 1.0f, 0.0f } } };
const Vectorf VECTOR_W = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };

// like Vector::operator * (float)
NFE_INLINE Vector operator*(float a, const Vector& b);


} // namespace Math
} // namespace NFE

#ifdef NFE_USE_SSE
#include "SSE/Vector.hpp"
#else
#include "FPU/Vector.hpp"
#endif
