/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float4 class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Float3.hpp"


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
 * Structure for efficient (in terms of memory) 4D vector storing - unaligned version of Vector4 class.
 */
struct Float4
{
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

    NFE_INLINE constexpr Float4();
    NFE_INLINE constexpr Float4(const Float2& rhs);
    NFE_INLINE constexpr Float4(const Float3& rhs);
    NFE_INLINE constexpr explicit Float4(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    NFE_INLINE constexpr explicit Float4(const float* src);

    NFE_INLINE operator Float2() const;
    NFE_INLINE operator Float3() const;

    /// element access
    NFE_INLINE float Get(uint32 index) const;
    NFE_INLINE float& Get(uint32 index);

    /// elements manipulations
    NFE_INLINE constexpr Float4 SplatX() const;
    NFE_INLINE constexpr Float4 SplatY() const;
    NFE_INLINE constexpr Float4 SplatZ() const;
    NFE_INLINE constexpr Float4 SplatW() const;
    NFE_INLINE constexpr static Float4 Splat(float f);

    template<bool negX = false, bool negY = false, bool negZ = false, bool negW = false>
    NFE_INLINE constexpr Float4 ChangeSign() const;

    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_INLINE Float4 Swizzle() const;

    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_INLINE static constexpr Float4 Blend(const Float4& a, const Float4& b);

    NFE_INLINE static Float4 SelectBySign(const Float4& a, const Float4& b, const Float4& sel);

    /// simple arithmetics
    NFE_INLINE constexpr Float4 operator- () const;
    NFE_INLINE constexpr Float4 operator+ (const Float4& b) const;
    NFE_INLINE constexpr Float4 operator- (const Float4& b) const;
    NFE_INLINE constexpr Float4 operator* (const Float4& b) const;
    NFE_INLINE Float4 operator/ (const Float4& b) const;
    NFE_INLINE constexpr Float4 operator* (float b) const;
    NFE_INLINE Float4 operator/ (float b) const;
    NFE_INLINE Float4& operator+= (const Float4& b);
    NFE_INLINE Float4& operator-= (const Float4& b);
    NFE_INLINE Float4& operator*= (const Float4& b);
    NFE_INLINE Float4& operator/= (const Float4& b);
    NFE_INLINE Float4& operator*= (float b);
    NFE_INLINE Float4& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE constexpr bool operator== (const Float4& b) const;
    NFE_INLINE constexpr bool operator< (const Float4& b) const;
    NFE_INLINE constexpr bool operator<= (const Float4& b) const;
    NFE_INLINE constexpr bool operator> (const Float4& b) const;
    NFE_INLINE constexpr bool operator>= (const Float4& b) const;
    NFE_INLINE constexpr bool operator!= (const Float4& b) const;

    /// Misc math
    NFE_INLINE static Float4 Floor(const Float4& v);
    NFE_INLINE static Float4 Sqrt(const Float4& v);
    NFE_INLINE static Float4 Reciprocal(const Float4& v);
    NFE_INLINE static constexpr Float4 Lerp(const Float4& v1, const Float4& v2, const Float4& weight);
    NFE_INLINE static constexpr Float4 Lerp(const Float4& v1, const Float4& v2, float weight);
    NFE_INLINE static constexpr Float4 Min(const Float4& a, const Float4& b);
    NFE_INLINE static constexpr Float4 Max(const Float4& a, const Float4& b);
    NFE_INLINE static constexpr Float4 Abs(const Float4& v);
    NFE_INLINE static constexpr bool AlmostEqual(const Float4& v1, const Float4& v2, float epsilon = NFE_MATH_EPSILON);

    /// Geometry
    NFE_INLINE static constexpr float Dot(const Float4& a, const Float4& b);
    NFE_INLINE static constexpr Float4 Cross(const Float4& a, const Float4& b, const Float4& c);
    NFE_INLINE float Length() const;
    NFE_INLINE Float4& Normalize();
    NFE_INLINE Float4 Normalized() const;
    NFE_INLINE static Float4 Reflect(const Float4& i, const Float4& n);
};

NFE_INLINE Float4 operator * (float a, const Float4& b);


} // namespace Math
} // namespace NFE


// enable all warnings again
#if defined(WIN32)
#pragma warning(pop)
#elif defined(__LINUX__) || defined(__linux__)
#pragma GCC diagnostic pop
#endif // defined(WIN32)


// definitions go here:
#include "Float4Impl.hpp"
