#pragma once

#include "Math.hpp"
#include "Vec2f.hpp"

namespace NFE {
namespace Math {

/**
 * Structure for efficient (in terms of memory) 3D vector storing.
 */
struct Vec3f
{
    NFE_DECLARE_CLASS(Vec3f)

public:

    NFE_UNNAMED_STRUCT union
    {
        float f[3];

        NFE_UNNAMED_STRUCT struct
        {
            float x;
            float y;
            float z;
        };
    };

    NFE_INLINE constexpr Vec3f();
    NFE_INLINE explicit constexpr Vec3f(const Vec2f& rhs, const float z = 0.0f);
    NFE_INLINE constexpr explicit Vec3f(float s);
    NFE_INLINE constexpr Vec3f(float x, float y, float z);
    NFE_INLINE constexpr explicit Vec3f(const float* src);

    // cast to Vec2f (Z component is discarded)
    NFE_INLINE operator Vec2f() const;

    NFE_INLINE bool IsValid() const;

    /// element access
    NFE_INLINE float Get(uint32 index) const;
    NFE_INLINE float& Get(uint32 index);

    /// elements manipulations
    NFE_INLINE constexpr const Vec3f SplatX() const;
    NFE_INLINE constexpr const Vec3f SplatY() const;
    NFE_INLINE constexpr const Vec3f SplatZ() const;
    NFE_INLINE static constexpr const Vec3f Splat(float f);

    template<bool changeX, bool changeY, bool changeZ>
    NFE_INLINE constexpr const Vec3f ChangeSign() const;

    template<uint32 ix, uint32 iy, uint32 iz>
    NFE_INLINE const Vec3f Swizzle() const;

    template<uint32 ix, uint32 iy, uint32 iz>
    NFE_INLINE static constexpr const Vec3f Blend(const Vec3f& a, const Vec3f& b);

    NFE_INLINE static const Vec3f SelectBySign(const Vec3f& a, const Vec3f& b, const Vec3f& sel);

    /// simple arithmetics
    NFE_INLINE constexpr const Vec3f operator- () const;
    NFE_INLINE constexpr const Vec3f operator+ (const Vec3f& b) const;
    NFE_INLINE constexpr const Vec3f operator- (const Vec3f& b) const;
    NFE_INLINE constexpr const Vec3f operator* (const Vec3f& b) const;
    NFE_INLINE const Vec3f operator/ (const Vec3f& b) const;
    NFE_INLINE constexpr const Vec3f operator* (float b) const;
    NFE_INLINE const Vec3f operator/ (float b) const;
    NFE_INLINE Vec3f& operator+= (const Vec3f& b);
    NFE_INLINE Vec3f& operator-= (const Vec3f& b);
    NFE_INLINE Vec3f& operator*= (const Vec3f& b);
    NFE_INLINE Vec3f& operator/= (const Vec3f& b);
    NFE_INLINE Vec3f& operator*= (float b);
    NFE_INLINE Vec3f& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE constexpr bool operator== (const Vec3f& b) const;
    NFE_INLINE constexpr bool operator< (const Vec3f& b) const;
    NFE_INLINE constexpr bool operator<= (const Vec3f& b) const;
    NFE_INLINE constexpr bool operator> (const Vec3f& b) const;
    NFE_INLINE constexpr bool operator>= (const Vec3f& b) const;
    NFE_INLINE constexpr bool operator!= (const Vec3f& b) const;

    /// Misc math
    NFE_INLINE static const Vec3f Floor(const Vec3f& v);
    NFE_INLINE static const Vec3f Sqrt(const Vec3f& v);
    NFE_INLINE static const Vec3f Reciprocal(const Vec3f& v);
    NFE_INLINE static constexpr const Vec3f Lerp(const Vec3f& v1, const Vec3f& v2, const Vec3f& weight);
    NFE_INLINE static constexpr const Vec3f Lerp(const Vec3f& v1, const Vec3f& v2, float weight);
    NFE_INLINE static constexpr const Vec3f Min(const Vec3f& a, const Vec3f& b);
    NFE_INLINE static constexpr const Vec3f Max(const Vec3f& a, const Vec3f& b);
    NFE_INLINE static constexpr const Vec3f Abs(const Vec3f& v);
    NFE_INLINE static constexpr bool AlmostEqual(const Vec3f& v1, const Vec3f& v2, float epsilon = FLT_EPSILON);

    /// Geometry
    NFE_INLINE static constexpr float Dot(const Vec3f& a, const Vec3f& b);
    NFE_INLINE static constexpr const Vec3f Cross(const Vec3f& a, const Vec3f& b);
    NFE_INLINE float Length() const;
    NFE_INLINE Vec3f& Normalize();
    NFE_INLINE const Vec3f Normalized() const;
};

NFE_INLINE const Vec3f operator * (float a, const Vec3f& b);

} // namespace Math
} // namespace NFE

#include "Vec3fImpl.hpp"
