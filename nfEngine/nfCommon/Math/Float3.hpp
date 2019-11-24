#pragma once

#include "Math.hpp"
#include "Float2.hpp"

namespace NFE {
namespace Math {

/**
 * Structure for efficient (in terms of memory) 3D vector storing.
 */
struct Float3
{
    NFE_DECLARE_CLASS(Float3);

public:

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
    NFE_INLINE explicit constexpr Float3(const Float2& rhs, const float z = 0.0f);
    NFE_INLINE constexpr explicit Float3(float s);
    NFE_INLINE constexpr Float3(float x, float y, float z);
    NFE_INLINE constexpr explicit Float3(const float* src);

    // cast to Float2 (Z component is discarded)
    NFE_INLINE operator Float2() const;

    NFE_INLINE bool IsValid() const;

    /// element access
    NFE_INLINE float Get(uint32 index) const;
    NFE_INLINE float& Get(uint32 index);

    /// elements manipulations
    NFE_INLINE constexpr const Float3 SplatX() const;
    NFE_INLINE constexpr const Float3 SplatY() const;
    NFE_INLINE constexpr const Float3 SplatZ() const;
    NFE_INLINE static constexpr const Float3 Splat(float f);

    template<bool x, bool y, bool z>
    NFE_INLINE constexpr const Float3 ChangeSign() const;

    template<uint32 ix, uint32 iy, uint32 iz>
    NFE_INLINE const Float3 Swizzle() const;

    template<uint32 ix, uint32 iy, uint32 iz>
    NFE_INLINE static constexpr const Float3 Blend(const Float3& a, const Float3& b);

    NFE_INLINE static const Float3 SelectBySign(const Float3& a, const Float3& b, const Float3& sel);

    /// simple arithmetics
    NFE_INLINE constexpr const Float3 operator- () const;
    NFE_INLINE constexpr const Float3 operator+ (const Float3& b) const;
    NFE_INLINE constexpr const Float3 operator- (const Float3& b) const;
    NFE_INLINE constexpr const Float3 operator* (const Float3& b) const;
    NFE_INLINE const Float3 operator/ (const Float3& b) const;
    NFE_INLINE constexpr const Float3 operator* (float b) const;
    NFE_INLINE const Float3 operator/ (float b) const;
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
    NFE_INLINE static const Float3 Floor(const Float3& v);
    NFE_INLINE static const Float3 Sqrt(const Float3& v);
    NFE_INLINE static const Float3 Reciprocal(const Float3& v);
    NFE_INLINE static constexpr const Float3 Lerp(const Float3& v1, const Float3& v2, const Float3& weight);
    NFE_INLINE static constexpr const Float3 Lerp(const Float3& v1, const Float3& v2, float weight);
    NFE_INLINE static constexpr const Float3 Min(const Float3& a, const Float3& b);
    NFE_INLINE static constexpr const Float3 Max(const Float3& a, const Float3& b);
    NFE_INLINE static constexpr const Float3 Abs(const Float3& v);
    NFE_INLINE static constexpr bool AlmostEqual(const Float3& v1, const Float3& v2, float epsilon = FLT_EPSILON);

    /// Geometry
    NFE_INLINE static constexpr float Dot(const Float3& a, const Float3& b);
    NFE_INLINE static constexpr const Float3 Cross(const Float3& a, const Float3& b);
    NFE_INLINE float Length() const;
    NFE_INLINE Float3& Normalize();
    NFE_INLINE const Float3 Normalized() const;
};

NFE_INLINE const Float3 operator * (float a, const Float3& b);

} // namespace Math
} // namespace NFE

#include "Float3Impl.hpp"
