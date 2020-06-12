#include "PCH.h"
#include "NoiseTexture3D.h"

namespace NFE {
namespace RT {

using namespace Math;

namespace utils {

} // namespace

NoiseTexture3D::NoiseTexture3D(const Math::Vec4f& colorA, const Math::Vec4f& colorB, const uint32 numOctaves)
    : mColorA(colorA)
    , mColorB(colorB)
    , mNumOctaves(numOctaves)
{
    NFE_ASSERT(colorA.IsValid(), "");
    NFE_ASSERT(colorB.IsValid(), "");
    NFE_ASSERT(numOctaves > 0 && numOctaves <= 20, "");

    // 1/sum(1 / 2^n), n = 0 ... mNumOctaves-1
    mScale = 1.0f / (2.0f - powf(2.0f, 1.0f - mNumOctaves));
}

const char* NoiseTexture3D::GetName() const
{
    return "noise3d";
}

static const Vec4f Hash33(Vec4f p3)
{
    p3 = Vec4f::Fmod1(p3 * Vec4f(0.1031f, 0.1030f, 0.0973f));
    p3 += Vec4f::Dot3V(p3, p3.Swizzle<1,0,2,3>() + Vec4f(33.33f));
    return Vec4f::Fmod1((p3.Swizzle<0,0,1,1>() + p3.Swizzle<1,0,0,3>()) * p3.Swizzle<2,1,0,3>());
}

float NoiseTexture3D::EvaluateInternal(const Math::Vec4f& coords)
{
    // taken from: https://www.shadertoy.com/view/XsX3zB

    // 1. find current tetrahedron T and it's four vertices
    // s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices
    // x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices

    const Vec4f s = Vec4f::Floor(coords + Vec4f::Dot3V(coords, Vec4f(F3)));
    const Vec4f x = coords - s + Vec4f::Dot3V(s, Vec4f(G3));

    const Vec4f e = Vec4f::Select(Vec4f(1.0f), Vec4f::Zero(), x < x.Swizzle<1,2,0,3>());
    const Vec4f i1 = e * (Vec4f(1.0f) - e.Swizzle<2,0,1,3>());
    const Vec4f i2 = Vec4f(1.0f) - e.Swizzle<2,0,1,3>() * (Vec4f(1.0f) - e);

    const Vec4f x1 = x + Vec4f(G3) - i1;
    const Vec4f x2 = x + Vec4f(2.0f * G3) - i2;
    const Vec4f x3 = x + Vec4f(3.0f * G3 - 1.0f);

    // 2. find four surflets and store them in d
    Vec4f w, d;

    // calculate surflet weights
    w.x = Vec4f::Dot3(x, x);
    w.y = Vec4f::Dot3(x1, x1);
    w.z = Vec4f::Dot3(x2, x2);
    w.w = Vec4f::Dot3(x3, x3);

    // w fades from 0.6 at the center of the surflet to 0.0 at the margin
    w = Vec4f::Max(Vec4f(0.6f) - w, Vec4f::Zero());

    // calculate surflet components
    d.x = Vec4f::Dot3(Hash33(s) - Vec4f(0.5f), x);
    d.y = Vec4f::Dot3(Hash33(s + i1) - Vec4f(0.5f), x1);
    d.z = Vec4f::Dot3(Hash33(s + i2) - Vec4f(0.5f), x2);
    d.w = Vec4f::Dot3(Hash33(s + Vec4f(1.0f)) - Vec4f(0.5f), x3);

    // multiply d by w^4
    w *= w;
    w *= w;
    d *= w;

    // 3. return the sum of the four surflets
    const float v = Vec4f::Dot4(d, Vec4f(52.0f));
    NFE_ASSERT(IsValid(v), "");

    return v;
}

const Vec4f NoiseTexture3D::Evaluate(const Vec4f& coords) const
{
    float value = 0.0f;

    float octaveValueScale = 1.0f;
    float octaveCoordScale = 1.0f;
    for (uint32 i = 0; i < mNumOctaves; ++i)
    {
        value += octaveValueScale * EvaluateInternal(coords * octaveCoordScale);
        octaveValueScale *= 0.5f;
        octaveCoordScale *= 2.0f;
    }

    value = Clamp(0.5f + 0.5f * value, 0.0f, 1.0f);

    // HACK
    value = Step(0.4f, 0.5f, value);

    return mScale * Vec4f::Lerp(mColorA, mColorB, value);
}

const Vec4f NoiseTexture3D::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
{
    float pdf = 1.0f;
    
    outCoords = Vec4f(u);

    if (outPdf)
    {
        *outPdf = pdf;
    }

    return NoiseTexture3D::Evaluate(outCoords);
}

} // namespace RT
} // namespace NFE
