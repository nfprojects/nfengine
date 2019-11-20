#include "PCH.h"
#include "NoiseTexture3D.h"
#include "../../nfCommon/Math/VectorInt4.hpp"

namespace NFE {
namespace RT {

using namespace Math;

namespace utils {

} // namespace

NoiseTexture3D::NoiseTexture3D(const Math::Vector4& colorA, const Math::Vector4& colorB, const uint32 numOctaves)
    : mColorA(colorA)
    , mColorB(colorB)
    , mNumOctaves(numOctaves)
{
    NFE_ASSERT(colorA.IsValid());
    NFE_ASSERT(colorB.IsValid());
    NFE_ASSERT(numOctaves > 0 && numOctaves <= 20);

    // 1/sum(1 / 2^n), n = 0 ... mNumOctaves-1
    mScale = 1.0f / (2.0f - powf(2.0f, 1.0f - mNumOctaves));
}

const char* NoiseTexture3D::GetName() const
{
    return "noise3d";
}

static const Vector4 Hash33(Vector4 p3)
{
    p3 = Vector4::Fmod1(p3 * Vector4(0.1031f, 0.1030f, 0.0973f));
    p3 += Vector4::Dot3V(p3, p3.Swizzle<1,0,2,3>() + Vector4(33.33f));
    return Vector4::Fmod1((p3.Swizzle<0,0,1,1>() + p3.Swizzle<1,0,0,3>()) * p3.Swizzle<2,1,0,3>());
}

float NoiseTexture3D::EvaluateInternal(const Math::Vector4& coords)
{
    // taken from: https://www.shadertoy.com/view/XsX3zB

    // 1. find current tetrahedron T and it's four vertices
    // s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices
    // x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices

    const Vector4 s = Vector4::Floor(coords + Vector4::Dot3V(coords, Vector4(F3)));
    const Vector4 x = coords - s + Vector4::Dot3V(s, Vector4(G3));

    const Vector4 e = Vector4::Select(Vector4(1.0f), Vector4::Zero(), x < x.Swizzle<1,2,0,3>());
    const Vector4 i1 = e * (Vector4(1.0f) - e.Swizzle<2,0,1,3>());
    const Vector4 i2 = Vector4(1.0f) - e.Swizzle<2,0,1,3>() * (Vector4(1.0f) - e);

    const Vector4 x1 = x + Vector4(G3) - i1;
    const Vector4 x2 = x + Vector4(2.0f * G3) - i2;
    const Vector4 x3 = x + Vector4(3.0f * G3 - 1.0f);

    // 2. find four surflets and store them in d
    Vector4 w, d;

    // calculate surflet weights
    w.x = Vector4::Dot3(x, x);
    w.y = Vector4::Dot3(x1, x1);
    w.z = Vector4::Dot3(x2, x2);
    w.w = Vector4::Dot3(x3, x3);

    // w fades from 0.6 at the center of the surflet to 0.0 at the margin
    w = Vector4::Max(Vector4(0.6f) - w, Vector4::Zero());

    // calculate surflet components
    d.x = Vector4::Dot3(Hash33(s) - Vector4(0.5f), x);
    d.y = Vector4::Dot3(Hash33(s + i1) - Vector4(0.5f), x1);
    d.z = Vector4::Dot3(Hash33(s + i2) - Vector4(0.5f), x2);
    d.w = Vector4::Dot3(Hash33(s + Vector4(1.0f)) - Vector4(0.5f), x3);

    // multiply d by w^4
    w *= w;
    w *= w;
    d *= w;

    // 3. return the sum of the four surflets
    const float v = Vector4::Dot4(d, Vector4(52.0f));
    NFE_ASSERT(IsValid(v));

    return v;
}

const Vector4 NoiseTexture3D::Evaluate(const Vector4& coords) const
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

    return mScale * Vector4::Lerp(mColorA, mColorB, value);
}

const Vector4 NoiseTexture3D::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    float pdf = 1.0f;
    
    outCoords = Vector4(u);

    if (outPdf)
    {
        *outPdf = pdf;
    }

    return NoiseTexture3D::Evaluate(outCoords);
}

} // namespace RT
} // namespace NFE
