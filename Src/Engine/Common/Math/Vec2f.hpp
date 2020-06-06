#pragma once

#include "Math.hpp"
#include "../Reflection/ReflectionClassDeclare.hpp"


namespace NFE {
namespace Math {

/**
 * Structure for efficient (in terms of memory) 2D vector storing.
 */
struct Vec2f
{
    NFE_DECLARE_CLASS(Vec2f)

public:

    NFE_UNNAMED_STRUCT union
    {
        float f[2];

        NFE_UNNAMED_STRUCT struct
        {
            float x;
            float y;
        };
    };

    NFE_INLINE constexpr Vec2f();
    NFE_INLINE constexpr explicit Vec2f(float v);
    NFE_INLINE constexpr Vec2f(float x, float y);
    NFE_INLINE constexpr explicit Vec2f(const float* src);

    NFE_INLINE bool IsValid() const;

    /// element access
    NFE_INLINE float Get(uint32 index) const;
    NFE_INLINE float& Get(uint32 index);

    /// elements manipulations
    NFE_INLINE constexpr const Vec2f SplatX() const;
    NFE_INLINE constexpr const Vec2f SplatY() const;
    NFE_INLINE constexpr static const Vec2f Splat(float f);

    template<bool changeX, bool changeY>
    NFE_INLINE constexpr const Vec2f ChangeSign() const;

    template<uint32 ix, uint32 iy>
    NFE_INLINE const Vec2f Swizzle() const;

    template<uint32 ix, uint32 iy>
    NFE_INLINE static constexpr const Vec2f Blend(const Vec2f& a, const Vec2f& b);

    NFE_INLINE static const Vec2f SelectBySign(const Vec2f& a, const Vec2f& b, const Vec2f& sel);

    /// simple arithmetics
    NFE_INLINE constexpr const Vec2f operator- () const;
    NFE_INLINE constexpr const Vec2f operator+ (const Vec2f& b) const;
    NFE_INLINE constexpr const Vec2f operator- (const Vec2f& b) const;
    NFE_INLINE constexpr const Vec2f operator* (const Vec2f& b) const;
    NFE_INLINE const Vec2f operator/ (const Vec2f& b) const;
    NFE_INLINE constexpr const Vec2f operator* (float b) const;
    NFE_INLINE const Vec2f operator/ (float b) const;
    NFE_INLINE Vec2f& operator+= (const Vec2f& b);
    NFE_INLINE Vec2f& operator-= (const Vec2f& b);
    NFE_INLINE Vec2f& operator*= (const Vec2f& b);
    NFE_INLINE Vec2f& operator/= (const Vec2f& b);
    NFE_INLINE Vec2f& operator*= (float b);
    NFE_INLINE Vec2f& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE constexpr bool operator== (const Vec2f& b) const;
    NFE_INLINE constexpr bool operator< (const Vec2f& b) const;
    NFE_INLINE constexpr bool operator<= (const Vec2f& b) const;
    NFE_INLINE constexpr bool operator> (const Vec2f& b) const;
    NFE_INLINE constexpr bool operator>= (const Vec2f& b) const;
    NFE_INLINE constexpr bool operator!= (const Vec2f& b) const;

    /// Misc math
    NFE_INLINE static const Vec2f Floor(const Vec2f& v);
    NFE_INLINE static const Vec2f Sqrt(const Vec2f& v);
    NFE_INLINE static const Vec2f Reciprocal(const Vec2f& v);
    NFE_INLINE static constexpr const Vec2f Lerp(const Vec2f& v1, const Vec2f& v2, const Vec2f& weight);
    NFE_INLINE static constexpr const Vec2f Lerp(const Vec2f& v1, const Vec2f& v2, float weight);
    NFE_INLINE static constexpr const Vec2f Min(const Vec2f& a, const Vec2f& b);
    NFE_INLINE static constexpr const Vec2f Max(const Vec2f& a, const Vec2f& b);
    NFE_INLINE static constexpr const Vec2f Abs(const Vec2f& v);
    NFE_INLINE static constexpr bool AlmostEqual(const Vec2f& v1, const Vec2f& v2, float epsilon = FLT_EPSILON);

    /// Geometry
    NFE_INLINE static constexpr float Dot(const Vec2f& a, const Vec2f& b);
    NFE_INLINE static constexpr float Cross(const Vec2f& a, const Vec2f& b);
    NFE_INLINE float Length() const;
    NFE_INLINE Vec2f& Normalize();
    NFE_INLINE const Vec2f Normalized() const;
};

NFE_INLINE const Vec2f operator * (float a, const Vec2f& b);


} // namespace Math
} // namespace NFE


#include "Vec2fImpl.hpp"
