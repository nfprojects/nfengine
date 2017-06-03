/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float3 class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Float2.hpp"


namespace NFE {
namespace Math {


// workaround for C++ nonstandard extension: nameless struct
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4201)
#elif defined(__LINUX__) || defined(__linux__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif // WIN32


/**
 * Structure for efficient (in terms of memory) 4D vector storing - unaligned version of Vector class.
 */
struct Float3
{
    union
    {
        float f[3];

        struct
        {
            float x;
            float y;
            float z;
        };
    };

    NFE_INLINE constexpr Float3();
    NFE_INLINE constexpr Float3(const Float2& rhs);
    NFE_INLINE constexpr explicit Float3(float x, float y = 0.0f, float z = 0.0f);
    NFE_INLINE constexpr explicit Float3(const float* src);

    // cast to Float2 (Z component is discarded)
    NFE_INLINE operator Float2() const;

    /// element access
    NFE_INLINE float Get(uint32 index) const;
    NFE_INLINE float& Get(uint32 index);

    /// elements manipulations
    NFE_INLINE constexpr Float3 SplatX() const;
    NFE_INLINE constexpr Float3 SplatY() const;
    NFE_INLINE constexpr Float3 SplatZ() const;
    NFE_INLINE constexpr static Float3 Splat(float f);

    template<bool x, bool y, bool z>
    NFE_INLINE constexpr Float3 ChangeSign() const;

    template<uint32 ix, uint32 iy, uint32 iz>
    NFE_INLINE Float3 Swizzle() const;

    template<uint32 ix, uint32 iy, uint32 iz>
    NFE_INLINE static constexpr Float3 Blend(const Float3& a, const Float3& b);

    NFE_INLINE static Float3 SelectBySign(const Float3& a, const Float3& b, const Float3& sel);

    /// simple arithmetics
    NFE_INLINE constexpr Float3 operator- () const;
    NFE_INLINE constexpr Float3 operator+ (const Float3& b) const;
    NFE_INLINE constexpr Float3 operator- (const Float3& b) const;
    NFE_INLINE constexpr Float3 operator* (const Float3& b) const;
    NFE_INLINE Float3 operator/ (const Float3& b) const;
    NFE_INLINE constexpr Float3 operator* (float b) const;
    NFE_INLINE Float3 operator/ (float b) const;
    NFE_INLINE Float3& operator+= (const Float3& b);
    NFE_INLINE Float3& operator-= (const Float3& b);
    NFE_INLINE Float3& operator*= (const Float3& b);
    NFE_INLINE Float3& operator/= (const Float3& b);
    NFE_INLINE Float3& operator*= (float b);
    NFE_INLINE Float3& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE constexpr bool operator== (const Float3& b) const;
    NFE_INLINE constexpr bool operator< (const Float3& b) const;
    NFE_INLINE constexpr bool operator<= (const Float3& b) const;
    NFE_INLINE constexpr bool operator> (const Float3& b) const;
    NFE_INLINE constexpr bool operator>= (const Float3& b) const;
    NFE_INLINE constexpr bool operator!= (const Float3& b) const;

    /// Misc math
    NFE_INLINE static Float3 Floor(const Float3& v);
    NFE_INLINE static Float3 Sqrt(const Float3& v);
    NFE_INLINE static Float3 Reciprocal(const Float3& v);
    NFE_INLINE static constexpr Float3 Lerp(const Float3& v1, const Float3& v2, const Float3& weight);
    NFE_INLINE static constexpr Float3 Lerp(const Float3& v1, const Float3& v2, float weight);
    NFE_INLINE static constexpr Float3 Min(const Float3& a, const Float3& b);
    NFE_INLINE static constexpr Float3 Max(const Float3& a, const Float3& b);
    NFE_INLINE static constexpr Float3 Abs(const Float3& v);
    NFE_INLINE static constexpr bool AlmostEqual(const Float3& v1, const Float3& v2, float epsilon = NFE_MATH_EPSILON);

    /// Geometry
    NFE_INLINE static constexpr float Dot(const Float3& a, const Float3& b);
    NFE_INLINE static constexpr Float3 Cross(const Float3& a, const Float3& b);
    NFE_INLINE float Length() const;
    NFE_INLINE Float3& Normalize();
    NFE_INLINE Float3 Normalized() const;
    NFE_INLINE static Float3 Reflect(const Float3& i, const Float3& n);
};

NFE_INLINE Float3 operator * (float a, const Float3& b);


} // namespace Math
} // namespace NFE


// enable all warnings again
#if defined(WIN32)
#pragma warning(pop)
#elif defined(__LINUX__) || defined(__linux__)
#pragma GCC diagnostic pop
#endif // defined(WIN32)


// definitions go here:
#include "Float3Impl.hpp"
