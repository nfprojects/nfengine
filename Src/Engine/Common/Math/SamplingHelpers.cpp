#include "PCH.hpp"
#include "SamplingHelpers.hpp"
#include "Transcendental.hpp"
#include "Vec8i.hpp"

namespace NFE {
namespace Math {

const Vec2x8f SamplingHelpers::GetCircle_Simd8(const Vec2x8f& u)
{
    // angle (uniform distribution)
    const Vec8f theta = u.x * (2.0f * NFE_MATH_PI);

    // radius (corrected distribution)
    const Vec8f r = Vec8f::Sqrt(u.y);

    const Vec8f vSin = Sin(theta);
    const Vec8f vCos = Sin(theta + Vec8f(NFE_MATH_PI / 2.0f));

    return { r * vSin, r * vCos };
}

const Vec4f SamplingHelpers::GetTriangle(const Vec2f u)
{
    const float t = sqrtf(u.x);
    return { 1.0f - t, u.y * t };
}

const Vec4f SamplingHelpers::GetCircle(const Vec2f u)
{
    // angle (uniform distribution)
    const float theta = 2.0f * NFE_MATH_PI * u.x;

    // radius (corrected distribution)
    const float r = sqrtf(u.y);

    return r * SinCos(theta);
}

const Vec4f SamplingHelpers::GetHexagon(const Vec3f u)
{
    constexpr Vec2f hexVectors[] =
    {
        { -1.0f, 0.0f },
        { 0.5f, 0.8660254f }, // sqrt(3.0f) / 2.0f
        { 0.5f, -0.8660254f }, // sqrt(3.0f) / 2.0f
        { -1.0f, 0.0f },
    };

    const uint32 x = static_cast<uint32>(3.0f * u.z);
    NFE_ASSERT(x < 3);

    const Vec2f a = hexVectors[x];
    const Vec2f b = hexVectors[x + 1];

    return Vec4f(u.x * a.x + u.y * b.x, u.x * a.y + u.y * b.y, 0.0f, 0.0f);
}

/*
const Vec4f SamplingHelpers::GetRegularPolygon(const uint32 n, const Vec4f& u)
{
    NFE_ASSERT(n >= 3, "Polygon must have at least 3 sides");

    // generate random point in a generic triangle
    const Vec2f uv = GetVec4f().ToVec2f();
    const float u = sqrtf(uv.x);
    const Vec2f triangle(1.0f - u, uv.y * u);

    // base triangle size
    const float a = Sin(NFE_MATH_PI / (float)n); // can be precomputed
    const float b = sqrtf(1.0f - a * a);

    // genrate point in base triangle
    const float sign = GetInt() % 2 ? 1.0f : -1.0f;
    const Vec4f base(b * (triangle.x + triangle.y), a * triangle.y * sign, 0.0f, 0.0f);

    // rotate
    const float alpha = RT_2PI * (float)(GetInt() % n) / (float)n;
    const Vec4f sinCosAlpha = SinCos(alpha);

    return Vec4f(sinCosAlpha.y * base.x - sinCosAlpha.x * base.y, sinCosAlpha.y * base.y + sinCosAlpha.x * base.x, 0.0f, 0.0f);
}

const Vec2x8f SamplingHelpers::GetRegularPolygon_Simd8(const uint32 n, const Vec2x8f& u)
{
    NFE_ASSERT(n >= 3, "Polygon must have at least 3 sides");

    const float invN = 1.0f / (float)n;

    // generate random point in a generic triangle
    const Vec8f t = Vec8f::Sqrt(u.x);
    const Vec2x8f triangle(Vec8f(1.0f) - t, u.y * t);

    // base triangle size
    const float a = Sin(NFE_MATH_PI * invN); // can be precomputed
    const float b = sqrtf(1.0f - a * a);

    // genrate point in base triangle
    const float sign = GetInt() % 2 ? 1.0f : -1.0f;
    const Vec2x8f base(b * (triangle.x + triangle.y), a * triangle.y * sign);

    // rotate
    const Vec8i i = (u.w & Vec8i(0x7FFFFFFF)) % n;
    const Vec8f alpha = i.ConvertToFloat() * (RT_2PI * invN);
    const Vec8f sinAlpha = Sin(alpha);
    const Vec8f cosAlpha = Cos(alpha);

    return Vec2x8f(cosAlpha * base.x - sinAlpha * base.y, cosAlpha * base.y + sinAlpha * base.x);
}
*/

const Vec4f SamplingHelpers::GetSphere(const Vec2f u)
{
    // based on http://mathworld.wolfram.com/SpherePointPicking.html

    // TODO 'u' should be already bipolar
    const Vec4f v = Vec4f::MulAndSub(Vec4f(u), 2.0f, VECTOR_ONE);

    const float t = sqrtf(Max(0.0f, 1.0f - v.y * v.y));
    const float theta = NFE_MATH_PI * v.x;
    Vec4f result = t * SinCos(theta); // xy
    result.z = v.y;

    return result;
}

const Vec4f SamplingHelpers::GetHemishpere(const Vec2f u)
{
    Vec4f p = GetSphere(u);
    p.z = Abs(p.z);
    return p;
}

const Vec4f SamplingHelpers::GetHemishpereCos(const Vec2f u)
{
    const float theta = 2.0f * NFE_MATH_PI * u.y;
    const float r = sqrtf(u.x); // this is required for the result vector to be normalized

    Vec4f result = r * SinCos(theta); // xy
    result.z = sqrtf(1.0f - u.x);

    return result;
}

float SamplingHelpers::GetFloatNormal(const Vec2f u)
{
    // Box-Muller method (take single result)
    return sqrtf(-2.0f * FastLog(u.x)) * Sin(2.0f * NFE_MATH_PI * u.y);
}

const Vec4f SamplingHelpers::GetFloatNormal2(const Vec2f u)
{
    // Box-Muller method
    return sqrtf(-2.0f * FastLog(u.x)) * SinCos(2.0f * NFE_MATH_PI * u.y);
}

} // namespace Math
} // namespace NFE
