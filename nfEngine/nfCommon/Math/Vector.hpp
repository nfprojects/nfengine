/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Vector class declaration and definitions of inline functions.
 */

#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {

struct Vectorf;
struct Vectori;
struct Float2;
struct Float3;
struct Float4;

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
    NFE_INLINE Vector(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    NFE_INLINE Vector(int x, int y = 0, int z = 0, int w = 0);
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

/**
 * Structure for efficient 2D vector storing.
 */
struct Float2
{
    float x, y;

    Float2() : x(0.0f), y(0.0f) {}
    Float2(float x_, float y_) : x(x_), y(y_) {}
    Float2(float* data) : x(data[0]), y(data[1]) {}
    explicit Float2(const Vector& vec) : x(vec[0]), y(vec[1]) {}

    NFE_INLINE Float2 operator=(const Vector& vec)
    {
        x = vec[0];
        y = vec[1];
        return *this;
    }
};

/**
 * Structure for efficient 3D vector storing.
 */
struct Float3
{
    float x, y, z;

    Float3() : x(0.0f), y(0.0f), z(0.0f) {}
    Float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Float3(float* data) : x(data[0]), y(data[1]), z(data[2]) {}
    explicit Float3(const Vector& vec) : x(vec[0]), y(vec[1]), z(vec[2]) {}

    NFE_INLINE Float3 operator=(const Vector& vec)
    {
        x = vec[0];
        y = vec[1];
        z = vec[2];
        return *this;
    }
};

/**
 * Structure for efficient 4D vector storing - unaligned version of Vector class.
 */
struct Float4
{
    float x, y, z, w;

    Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Float4(float* data) : x(data[0]), y(data[1]), z(data[2]), w(data[3]) {}
    explicit Float4(const Vector& vec) : x(vec[0]), y(vec[1]), z(vec[2]), w(vec[3]) {}

    NFE_INLINE Float4& operator=(const Vector& vec)
    {
        x = vec[0];
        y = vec[1];
        z = vec[2];
        w = vec[3];
        return *this;
    }
};


//
// Constants definitions
//

const Vectorf VECTOR_EPSILON = {{{NFE_MATH_EPSILON, NFE_MATH_EPSILON,
                             NFE_MATH_EPSILON, NFE_MATH_EPSILON}}};
const Vectorf VECTOR_ONE = {{{1.0f, 1.0f, 1.0f, 1.0f}}};
const Vectorf VECTOR_ONE3 = {{{1.0f, 1.0f, 1.0f, 0.0f}}};
const Vectorf VECTOR_MINUS_ONE = {{{-1.0f, -1.0f, -1.0f, -1.0f}}};
const Vectori VECTOR_MASK_X = {{{0xFFFFFFFF, 0, 0, 0}}};
const Vectori VECTOR_MASK_Y = {{{0, 0xFFFFFFFF, 0, 0}}};
const Vectori VECTOR_MASK_Z = {{{0, 0, 0xFFFFFFFF, 0}}};
const Vectori VECTOR_MASK_W = {{{0, 0, 0, 0xFFFFFFFF}}};
const Vectori VECTOR_MASK_XYZ = {{{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0}}};
const Vectori VECTOR_MASK_ABS = {{{0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF}}};

const Vectorf VECTOR_INV_255 = {{{1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f}}};
const Vectorf VECTOR_255 = {{{255.0f, 255.0f, 255.0f, 255.0f}}};

/// identity matrix rows
const Vectorf VECTOR_IDENTITY_ROW_0 = {{{1.0f, 0.0f, 0.0f, 0.0f}}};
const Vectorf VECTOR_IDENTITY_ROW_1 = {{{0.0f, 1.0f, 0.0f, 0.0f}}};
const Vectorf VECTOR_IDENTITY_ROW_2 = {{{0.0f, 0.0f, 1.0f, 0.0f}}};
const Vectorf VECTOR_IDENTITY_ROW_3 = {{{0.0f, 0.0f, 0.0f, 1.0f}}};


//
// Function declarations
//

/**
 * Convert 4 unsigned chars to a Vector.
 */
NFE_INLINE Vector VectorLoadUChar4(const unsigned char* src);

/**
 * Convert a Vector to 4 unsigned chars.
 */
NFE_INLINE void VectorStoreUChar4(const Vector& src, unsigned char* dest);
NFE_INLINE void VectorStore(const Vector& src, float* dest);
NFE_INLINE void VectorStore(const Vector& src, Float2* dest);
NFE_INLINE void VectorStore(const Vector& src, Float3* dest);
NFE_INLINE void VectorStore(const Vector& src, Float4* dest);
NFE_INLINE Vector VectorSplat(float f);

NFE_INLINE Vector operator*(float a, const Vector& b);
NFE_INLINE Vector VectorFloor(const Vector& v);
NFE_INLINE Vector VectorSqrt(const Vector& v);
NFE_INLINE Vector VectorSqrt4(const Vector& v);
NFE_INLINE Vector VectorReciprocal(const Vector& v);
NFE_INLINE Vector VectorLerp(const Vector& v1, const Vector& v2, const Vector& weight);
NFE_INLINE Vector VectorLerp(const Vector& v1, const Vector& v2, float weight);
NFE_INLINE Vector VectorMin(const Vector& a, const Vector& b);
NFE_INLINE Vector VectorMax(const Vector& a, const Vector& b);
NFE_INLINE Vector VectorAbs(const Vector& v);

NFE_INLINE int VectorEqualMask(const Vector& v1, const Vector& v2);
NFE_INLINE int VectorLessMask(const Vector& v1, const Vector& v2);
NFE_INLINE int VectorLessEqMask(const Vector& v1, const Vector& v2);
NFE_INLINE int VectorGreaterMask(const Vector& v1, const Vector& v2);
NFE_INLINE int VectorGreaterEqMask(const Vector& v1, const Vector& v2);
NFE_INLINE int VectorNotEqualMask(const Vector& v1, const Vector& v2);

NFE_INLINE bool VectorEqual2(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorLess2(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorLessEq2(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorGreater2(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorGreaterEq2(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorNotEqual2(const Vector& v1, const Vector& v2);

NFE_INLINE bool VectorEqual3(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorLess3(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorLessEq3(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorGreater3(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorGreaterEq3(const Vector& v1, const Vector& v2);
NFE_INLINE bool VectorNotEqual3(const Vector& v1, const Vector& v2);

/**
 * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
 */
NFE_INLINE Vector VectorSelectBySign(const Vector& a, const Vector& b, const Vector& sel);

/**
 * Calculate 3D dot product.
 * @return Dot product (scalar value).
 */
NFE_INLINE float VectorDot3f(const Vector& v1, const Vector& v2);

/**
 * Calculate 3D dot product.
 * @return Vector of dot products.
 */
NFE_INLINE Vector VectorDot3(const Vector& v1, const Vector& v2);

/**
 * Calculate 4D dot product.
 * @return Vector of dot products.
 */
NFE_INLINE Vector VectorDot4(const Vector& v1, const Vector& v2);

/**
 * Calculate 3D cross product.
 * @return Vector of dot products.
 */
NFE_INLINE Vector VectorCross3(const Vector& v1, const Vector& v2);

/**
 * Calculate length of a 3D vector.
 * @details 4th element is ignored.
 * @return Length of vector @p.
 */
NFE_INLINE float VectorLength3f(const Vector& v);

/**
 * Calculate length of a 3D vector.
 * @details 4th element is ignored.
 * @return Vector of @p v length.
 */
NFE_INLINE Vector VectorLength3(const Vector& v);

/**
 * Calculate length of a 4D vector.
 * @return Length of vector @p.
 */
NFE_INLINE Vector VectorLength4(const Vector& v);

/**
 * Calculate length of a 4D vector.
 * @return Length of vector @p.
 */
NFE_INLINE float VectorLength4f(const Vector& v);

/**
 * Calculate length of a 4D vector.
 * @return Vector of @p v length.
 */
NFE_INLINE Vector VectorLength4(const Vector& v);

/**
 * Normalize 3D vector.
 * @details 4th element is ignored.
 */
NFE_INLINE Vector VectorNormalize3(const Vector& v);

/**
 * Normalize 4D vector.
 */
NFE_INLINE Vector VectorNormalize4(const Vector& v);

/**
 * Reflect a 3D vector.
 * @param i incident vector
 * @param n normal vector
 * @details 4th element is ignored.
 */
NFE_INLINE Vector VectorReflect3(const Vector& i, const Vector& n);

/**
 * Generate a plane equation from 3 points.
 * @param p1,p2,p3 Planar points
 * @return Plane equation
 */
NFE_INLINE Vector PlaneFromPoints(const Vector& p1, const Vector& p2, const Vector& p3);

/**
 * Generate a plane equation from a normal and a point.
 * @return Plane equation
 */
NFE_INLINE Vector PlaneFromNormalAndPoint(const Vector& normal, const Vector& p);

/**
 * Determine plane side a point belongs to.
 * @return "true" - positive side, "false" - negative side
 */
NFE_INLINE bool PlanePointSide(const Vector& plane, const Vector& point);

} // namespace Math
} // namespace NFE

#ifdef NFE_USE_SSE
#include "SSE/Vector.hpp"
#else
#include "FPU/Vector.hpp"
#endif
