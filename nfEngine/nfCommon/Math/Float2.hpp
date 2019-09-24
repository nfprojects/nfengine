#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {

/**
 * Structure for efficient (in terms of memory) 2D vector storing.
 */
struct Float2
{
    union
    {
        float f[2];

        struct
        {
            float x;
            float y;
        };
    };

    NFE_INLINE constexpr Float2();
    NFE_INLINE constexpr explicit Float2(float v);
    NFE_INLINE constexpr Float2(float x, float y);
    NFE_INLINE constexpr explicit Float2(const float* src);

    NFE_INLINE bool IsValid() const;

    /// element access
    NFE_INLINE float Get(uint32 index) const;
    NFE_INLINE float& Get(uint32 index);

    /// elements manipulations
    NFE_INLINE constexpr const Float2 SplatX() const;
    NFE_INLINE constexpr const Float2 SplatY() const;
    NFE_INLINE constexpr static const Float2 Splat(float f);

    template<bool x, bool y>
    NFE_INLINE constexpr const Float2 ChangeSign() const;

    template<uint32 ix, uint32 iy>
    NFE_INLINE const Float2 Swizzle() const;

    template<uint32 ix, uint32 iy>
    NFE_INLINE static constexpr const Float2 Blend(const Float2& a, const Float2& b);

    NFE_INLINE static const Float2 SelectBySign(const Float2& a, const Float2& b, const Float2& sel);

    /// simple arithmetics
    NFE_INLINE constexpr const Float2 operator- () const;
    NFE_INLINE constexpr const Float2 operator+ (const Float2& b) const;
    NFE_INLINE constexpr const Float2 operator- (const Float2& b) const;
    NFE_INLINE constexpr const Float2 operator* (const Float2& b) const;
    NFE_INLINE const Float2 operator/ (const Float2& b) const;
    NFE_INLINE constexpr const Float2 operator* (float b) const;
    NFE_INLINE const Float2 operator/ (float b) const;
    NFE_INLINE Float2& operator+= (const Float2& b);
    NFE_INLINE Float2& operator-= (const Float2& b);
    NFE_INLINE Float2& operator*= (const Float2& b);
    NFE_INLINE Float2& operator/= (const Float2& b);
    NFE_INLINE Float2& operator*= (float b);
    NFE_INLINE Float2& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE constexpr bool operator== (const Float2& b) const;
    NFE_INLINE constexpr bool operator< (const Float2& b) const;
    NFE_INLINE constexpr bool operator<= (const Float2& b) const;
    NFE_INLINE constexpr bool operator> (const Float2& b) const;
    NFE_INLINE constexpr bool operator>= (const Float2& b) const;
    NFE_INLINE constexpr bool operator!= (const Float2& b) const;

    /// Misc math
    NFE_INLINE static const Float2 Floor(const Float2& v);
    NFE_INLINE static const Float2 Sqrt(const Float2& v);
    NFE_INLINE static const Float2 Reciprocal(const Float2& v);
    NFE_INLINE static constexpr const Float2 Lerp(const Float2& v1, const Float2& v2, const Float2& weight);
    NFE_INLINE static constexpr const Float2 Lerp(const Float2& v1, const Float2& v2, float weight);
    NFE_INLINE static constexpr const Float2 Min(const Float2& a, const Float2& b);
    NFE_INLINE static constexpr const Float2 Max(const Float2& a, const Float2& b);
    NFE_INLINE static constexpr const Float2 Abs(const Float2& v);
    NFE_INLINE static constexpr bool AlmostEqual(const Float2& v1, const Float2& v2, float epsilon = FLT_EPSILON);

    /// Geometry
    NFE_INLINE static constexpr float Dot(const Float2& a, const Float2& b);
    NFE_INLINE static constexpr float Cross(const Float2& a, const Float2& b);
    NFE_INLINE float Length() const;
    NFE_INLINE Float2& Normalize();
    NFE_INLINE const Float2 Normalized() const;
};

NFE_INLINE const Float2 operator * (float a, const Float2& b);


} // namespace Math
} // namespace NFE


#include "Float2Impl.hpp"
