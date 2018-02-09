/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Vector4 class declaration and definitions of inline functions.
 */

#pragma once

#include "Math.hpp"
#include "Float4.hpp"


namespace NFE {
namespace Math {

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4201)
#elif defined(__LINUX__) || defined(__linux__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif // WIN32

/**
 * 4 element vector of floats.
 * @details Used to perform operations on 4D, 3D and 2D vectors. The content is mapped to
 *          SSE registers to speedup computations.
 */
struct NFE_ALIGN(16) Vector4
{
    union
    {
        float f[4];
        int i[4];
        unsigned int u[4];

        struct
        {
            float x;
            float y;
            float z;
            float w;
        };

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
    NFE_INLINE Vector4(const __m128& src)
    {
        v = src;
    }
#endif

    /// constructors
    NFE_INLINE Vector4();
    NFE_INLINE explicit Vector4(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    NFE_INLINE explicit Vector4(int x, int y = 0, int z = 0, int w = 0);
    NFE_INLINE explicit Vector4(const float* src);
    NFE_INLINE explicit Vector4(const Float2& src);
    NFE_INLINE explicit Vector4(const Float3& src);
    NFE_INLINE explicit Vector4(const Float4& src);
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
    NFE_INLINE Vector4 operator- () const;
    NFE_INLINE Vector4 operator+ (const Vector4& b) const;
    NFE_INLINE Vector4 operator- (const Vector4& b) const;
    NFE_INLINE Vector4 operator* (const Vector4& b) const;
    NFE_INLINE Vector4 operator/ (const Vector4& b) const;
    NFE_INLINE Vector4 operator* (float b) const;
    NFE_INLINE Vector4 operator/ (float b) const;
    NFE_INLINE Vector4& operator+= (const Vector4& b);
    NFE_INLINE Vector4& operator-= (const Vector4& b);
    NFE_INLINE Vector4& operator*= (const Vector4& b);
    NFE_INLINE Vector4& operator/= (const Vector4& b);
    NFE_INLINE Vector4& operator*= (float b);
    NFE_INLINE Vector4& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE bool operator== (const Vector4& b) const;
    NFE_INLINE bool operator< (const Vector4& b) const;
    NFE_INLINE bool operator<= (const Vector4& b) const;
    NFE_INLINE bool operator> (const Vector4& b) const;
    NFE_INLINE bool operator>= (const Vector4& b) const;
    NFE_INLINE bool operator!= (const Vector4& b) const;

    /// bitwise logic operations
    NFE_INLINE Vector4 operator& (const Vector4& b) const;
    NFE_INLINE Vector4 operator| (const Vector4& b) const;
    NFE_INLINE Vector4 operator^ (const Vector4& b) const;
    NFE_INLINE Vector4& operator&= (const Vector4& b);
    NFE_INLINE Vector4& operator|= (const Vector4& b);
    NFE_INLINE Vector4& operator^= (const Vector4& b);

    NFE_INLINE Vector4 SplatX() const;
    NFE_INLINE Vector4 SplatY() const;
    NFE_INLINE Vector4 SplatZ() const;
    NFE_INLINE Vector4 SplatW() const;

    /**
     * Rearrange vector elements.
     */
    template<bool negX = false, bool negY = false, bool negZ = false, bool negW = false>
    NFE_INLINE Vector4 ChangeSign() const;

    /**
     * Rearrange vector elements.
     */
    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_INLINE Vector4 Swizzle() const;

    /**
     * Blend two vectors.
     */
    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_INLINE static Vector4 Blend(const Vector4& a, const Vector4& b);

    /**
     * Convert 4 uint8 to a Vector4.
     */
    NFE_INLINE static Vector4 Load4(const uint8* src);

    /**
     * Convert a Vector4 to 4 unsigned chars.
     */
    NFE_INLINE void Store4(uint8* dest) const;

    NFE_INLINE void Store(float* dest) const;
    NFE_INLINE void Store(Float2* dest) const;
    NFE_INLINE void Store(Float3* dest) const;
    NFE_INLINE void Store(Float4* dest) const;
    NFE_INLINE static Vector4 Splat(float f);

    NFE_INLINE static Vector4 Floor(const Vector4& v);
    NFE_INLINE static Vector4 Sqrt(const Vector4& v);
    NFE_INLINE static Vector4 Sqrt4(const Vector4& v);
    NFE_INLINE static Vector4 Reciprocal(const Vector4& v);
    NFE_INLINE static Vector4 Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight);
    NFE_INLINE static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float weight);
    NFE_INLINE static Vector4 Min(const Vector4& a, const Vector4& b);
    NFE_INLINE static Vector4 Max(const Vector4& a, const Vector4& b);
    NFE_INLINE static Vector4 Abs(const Vector4& v);

    NFE_INLINE static int EqualMask(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static int LessMask(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static int LessEqMask(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static int GreaterMask(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static int GreaterEqMask(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static int NotEqualMask(const Vector4& v1, const Vector4& v2);

    NFE_INLINE static bool Equal2(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool Less2(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool LessEq2(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool Greater2(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool GreaterEq2(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool NotEqual2(const Vector4& v1, const Vector4& v2);

    NFE_INLINE static bool Equal3(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool Less3(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool LessEq3(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool Greater3(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool GreaterEq3(const Vector4& v1, const Vector4& v2);
    NFE_INLINE static bool NotEqual3(const Vector4& v1, const Vector4& v2);

    /**
     * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
     */
    NFE_INLINE static Vector4 SelectBySign(const Vector4& a, const Vector4& b, const Vector4& sel);

    /**
     * Calculate 2D dot product.
     * @return Dot product (scalar value).
     */
    NFE_INLINE static float Dot2(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 2D dot product.
     * @return Vector4 of dot products.
     */
    NFE_INLINE static Vector4 Dot2V(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 3D dot product.
     * @return Dot product (scalar value).
     */
    NFE_INLINE static float Dot3(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 3D dot product.
     * @return Vector4 of dot products.
     */
    NFE_INLINE static Vector4 Dot3V(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 4D dot product.
     * @return Vector4 of dot products.
     */
    NFE_INLINE static float Dot4(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 4D dot product.
     * @return Dot product (scalar value).
     */
    NFE_INLINE static Vector4 Dot4V(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 3D cross product.
     * @return Vector4 of dot products.
     */
    NFE_INLINE static Vector4 Cross3(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate length of a 2D vector.
     * @return Length of vector @p.
     */
    NFE_INLINE float Length2() const;

    /**
     * Calculate length of a 2D vector.
     * @return Length of vector @p.
     */
    NFE_INLINE Vector4 Length2V() const;

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Length of vector @p.
     */
    NFE_INLINE float Length3() const;

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Vector4 of @p v length.
     */
    NFE_INLINE Vector4 Length3V() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    NFE_INLINE float Length4() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    NFE_INLINE Vector4 Length4V() const;

    /**
     * Normalize as 2D vector.
     * @details 3rd and 4th elements are ignored.
     */
    NFE_INLINE Vector4& Normalize2();

    /**
     * Normalize as 3D vector.
     * @details 4th element is ignored.
     */
    NFE_INLINE Vector4& Normalize3();

    /**
     * Normalize as 4D vector.
     */
    NFE_INLINE Vector4& Normalize4();

    /**
     * Return normalized 2D vector.
     * @details 3rd and 4th elements are ignored.
     */
    NFE_INLINE Vector4 Normalized2() const;

    /**
     * Return normalized 3D vector.
     * @details 4th element is ignored.
     */
    NFE_INLINE Vector4 Normalized3() const;

    /**
     * Return normalized 4D vector.
     */
    NFE_INLINE Vector4 Normalized4() const;

    /**
     * Reflect a 3D vector.
     * @param i incident vector
     * @param n normal vector
     * @details 4th element is ignored.
     */
    NFE_INLINE static Vector4 Reflect3(const Vector4& i, const Vector4& n);

    /**
     * Check if two vectors are (almost) equal.
     */
    NFE_INLINE static bool AlmostEqual(const Vector4& v1, const Vector4& v2, float epsilon = NFE_MATH_EPSILON)
    {
        Vector4 diff = Abs(v1 - v2);
        Vector4 epsilonV = Vector4::Splat(epsilon);
        return diff < epsilonV;
    }

    /**
     * Fused multiply and add.
     * @return  a * b + c
     */
    NFE_INLINE static Vector4 MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);

    /**
     * Fused multiply and subtract.
     * @return  a * b - c
     */
    NFE_INLINE static Vector4 MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);

    /**
     * Fused multiply (negated) and add.
     * @return  - a * b + c
     */
    NFE_INLINE static Vector4 NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);

    /**
     * Fused multiply (negated) and subtract.
     * @return  - a * b - c
     */
    NFE_INLINE static Vector4 NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);
};


// enable all warnings again
#if defined(WIN32)
#pragma warning(pop)
#elif defined(__LINUX__) || defined(__linux__)
#pragma GCC diagnostic pop
#endif // defined(WIN32)


// like Vector4::operator * (float)
NFE_INLINE Vector4 operator*(float a, const Vector4& b);


} // namespace Math
} // namespace NFE


#include "Vector4Constants.hpp"

#ifdef NFE_USE_SSE
#include "SSE/Vector4.hpp"
#else
#include "FPU/Vector4.hpp"
#endif
