/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Vec4fU class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vec3f.hpp"


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
 * Structure for efficient (in terms of memory) 4D vector storing - unaligned version of Vec4f class.
 */
struct Vec4fU
{
    NFE_DECLARE_CLASS(Vec4fU)

public:

    union
    {
        float f[4];

        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };

    NFE_INLINE constexpr Vec4fU();
    NFE_INLINE constexpr Vec4fU(const Vec2f& rhs);
    NFE_INLINE constexpr Vec4fU(const Vec3f& rhs);
    NFE_INLINE constexpr explicit Vec4fU(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    NFE_INLINE constexpr explicit Vec4fU(const float* src);

    NFE_INLINE operator Vec2f() const;
    NFE_INLINE operator Vec3f() const;

    /// element access
    NFE_INLINE float Get(uint32 index) const;
    NFE_INLINE float& Get(uint32 index);

    /// elements manipulations
    NFE_INLINE constexpr Vec4fU SplatX() const;
    NFE_INLINE constexpr Vec4fU SplatY() const;
    NFE_INLINE constexpr Vec4fU SplatZ() const;
    NFE_INLINE constexpr Vec4fU SplatW() const;
    NFE_INLINE constexpr static Vec4fU Splat(float f);

    template<bool negX = false, bool negY = false, bool negZ = false, bool negW = false>
    NFE_INLINE constexpr Vec4fU ChangeSign() const;

    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_INLINE Vec4fU Swizzle() const;

    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_INLINE static constexpr Vec4fU Blend(const Vec4fU& a, const Vec4fU& b);

    NFE_INLINE static Vec4fU SelectBySign(const Vec4fU& a, const Vec4fU& b, const Vec4fU& sel);

    /// simple arithmetics
    NFE_INLINE constexpr Vec4fU operator- () const;
    NFE_INLINE constexpr Vec4fU operator+ (const Vec4fU& b) const;
    NFE_INLINE constexpr Vec4fU operator- (const Vec4fU& b) const;
    NFE_INLINE constexpr Vec4fU operator* (const Vec4fU& b) const;
    NFE_INLINE Vec4fU operator/ (const Vec4fU& b) const;
    NFE_INLINE constexpr Vec4fU operator* (float b) const;
    NFE_INLINE Vec4fU operator/ (float b) const;
    NFE_INLINE Vec4fU& operator+= (const Vec4fU& b);
    NFE_INLINE Vec4fU& operator-= (const Vec4fU& b);
    NFE_INLINE Vec4fU& operator*= (const Vec4fU& b);
    NFE_INLINE Vec4fU& operator/= (const Vec4fU& b);
    NFE_INLINE Vec4fU& operator*= (float b);
    NFE_INLINE Vec4fU& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE constexpr bool operator== (const Vec4fU& b) const;
    NFE_INLINE constexpr bool operator< (const Vec4fU& b) const;
    NFE_INLINE constexpr bool operator<= (const Vec4fU& b) const;
    NFE_INLINE constexpr bool operator> (const Vec4fU& b) const;
    NFE_INLINE constexpr bool operator>= (const Vec4fU& b) const;
    NFE_INLINE constexpr bool operator!= (const Vec4fU& b) const;

    /// Misc math
    NFE_INLINE static Vec4fU Floor(const Vec4fU& v);
    NFE_INLINE static Vec4fU Sqrt(const Vec4fU& v);
    NFE_INLINE static Vec4fU Reciprocal(const Vec4fU& v);
    NFE_INLINE static constexpr Vec4fU Lerp(const Vec4fU& v1, const Vec4fU& v2, const Vec4fU& weight);
    NFE_INLINE static constexpr Vec4fU Lerp(const Vec4fU& v1, const Vec4fU& v2, float weight);
    NFE_INLINE static constexpr Vec4fU Min(const Vec4fU& a, const Vec4fU& b);
    NFE_INLINE static constexpr Vec4fU Max(const Vec4fU& a, const Vec4fU& b);
    NFE_INLINE static constexpr Vec4fU Abs(const Vec4fU& v);
    NFE_INLINE static constexpr bool AlmostEqual(const Vec4fU& v1, const Vec4fU& v2, float epsilon = NFE_MATH_EPSILON);

    /// Geometry
    NFE_INLINE static constexpr float Dot(const Vec4fU& a, const Vec4fU& b);
    NFE_INLINE static constexpr Vec4fU Cross(const Vec4fU& a, const Vec4fU& b, const Vec4fU& c);
    NFE_INLINE float Length() const;
    NFE_INLINE Vec4fU& Normalize();
    NFE_INLINE Vec4fU Normalized() const;
    NFE_INLINE static Vec4fU Reflect(const Vec4fU& i, const Vec4fU& n);
};

NFE_INLINE Vec4fU operator * (float a, const Vec4fU& b);


} // namespace Math
} // namespace NFE


// enable all warnings again
#if defined(WIN32)
#pragma warning(pop)
#elif defined(__LINUX__) || defined(__linux__)
#pragma GCC diagnostic pop
#endif // defined(WIN32)


// definitions go here:
#include "Vec4fUImpl.hpp"
