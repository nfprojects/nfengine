/**
 * @file
 * @brief  Definitions of transcendental functions.
 */

#include "PCH.hpp"
#include "Transcendental.hpp"
#include "Math.hpp"
#include "System\Assertion.hpp"


// TODO atan2


namespace NFE {
namespace Math {


namespace {

int32 FloatAsInt(const float f)
{
    Bits32 bits;
    bits.f = f;
    return bits.si;
}

float IntAsFloat(const int32 i)
{
    Bits32 bits;
    bits.si = i;
    return bits.f;
}

// calculates acos(|x|)
NFE_INLINE float ACosAbs(float x)
{
    // based on DirectXMath implementation:
    // https://github.com/Microsoft/DirectXMath/blob/master/Inc/DirectXMathMisc.inl

    x = fabsf(x);
    float root = sqrtf(1.0f - x);

    const float c0 =  1.5707963050f;
    const float c1 = -0.2145988016f;
    const float c2 =  0.0889789874f;
    const float c3 = -0.0501743046f;
    const float c4 =  0.0308918810f;
    const float c5 = -0.0170881256f;
    const float c6 =  0.0066700901f;
    const float c7 = -0.0012624911f;

    return root * (c0 + x * (c1 + x * (c2 + x * (c3 + x * (c4 + x * (c5 + x * (c6 + x * c7)))))));
}

} // namespace


float Sin(float x)
{
    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const int32 i = static_cast<int32>(x * (1.0f / Constants::pi<float>));
    x -= static_cast<float>(i) * Constants::pi<float>;

    const float x2 = x * x;

    const float c0 =  9.9999970197e-01f;
    const float c1 = -1.6666577756e-01f;
    const float c2 =  8.3325579762e-03f;
    const float c3 = -1.9812576647e-04f;
    const float c4 =  2.7040521217e-06f;
    const float c5 = -2.0532988642e-08f;

    float y = x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));

    return (i & 1) ? -y : y;
}

float Cos(float x)
{
    return Sin(x + Constants::pi<float> / 2.0f);
}

float ACos(float x)
{
    // NFE_ASSERT(x >= -1.0f && x <= 1.0f, "Invalid argument");

    bool nonnegative = (x >= 0.0f);
    const float acosAbs = ACosAbs(x);

    // acos(x) = pi - acos(-x) when x < 0
    return nonnegative ? acosAbs : Constants::pi<float> - acosAbs;
}

float ASin(float x)
{
    // NFE_ASSERT(x >= -1.0f && x <= 1.0f, "Invalid argument");

    bool nonnegative = (x >= 0.0f);
    const float acosAbs = ACosAbs(x);

    // acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
    return nonnegative ?
        Constants::pi<float> / 2.0f - acosAbs :
        acosAbs - Constants::pi<float> / 2.0f;
}

float ATan(float x)
{
    // based on:
    // https://stackoverflow.com/questions/26692859/best-machine-optimized-polynomial-minimax-approximation-to-arctangent-on-1-1

    float t = Abs(x);

    // range reduction
    float z = t;
    if (t > 1.0f)
    {
        z = 1.0f / z;
    }

    const float x2 = z * z;
    float y =    2.78569828e-3f;
    y = y * x2 - 1.58660226e-2f;
    y = y * x2 + 4.24722321e-2f;
    y = y * x2 - 7.49753043e-2f;
    y = y * x2 + 1.06448799e-1f;
    y = y * x2 - 1.42070308e-1f;
    y = y * x2 + 1.99934542e-1f;
    y = y * x2 - 3.33331466e-1f;
    y *= x2;
    y = y * z + z;

    // atan(x) = pi/2 - atan(1/x)
    if (t > 1.0f)
    {
        y = Constants::pi<float> / 2.0f - y;
    }

    return CopySign(y, x);
}

float Exp(float x)
{
    // implementation based on: "A more accurate, performance-competitive implementation of expf" by njuffa

    // handle special cases: severe overflow / underflow
    if (x >= 87.0f) // overflow
    {
        return std::numeric_limits<float>::infinity();
    }
    else if (x <= -87.0f) // underflow
    {
        return 0.0f;
    }


    float f, r;

    // exp(x) = 2**i * exp(f); i = rintf (x / log(2))
    r = fmaf(1.442695f, x, 12582912.f) - 12582912.f;
    f = -6.93145752e-1f * r + x;
    f = -1.42860677e-6f * r + f;
    const int32 i = static_cast<int32>(r);

    // approximate r = exp(f) on interval [-log(2)/2, +log(2)/2]
    r =         1.37853622e-3f;
    r = r * f + 8.37326515e-3f;
    r = r * f + 4.16694842e-2f;
    r = r * f + 1.66664705e-1f;
    r = r * f + 4.99999851e-1f;
    r = r * f + 1.0f;
    r = r * f + 1.0f;

    const int32 ia = (i > 0) ? 0 : 0x83000000;
    const float s = IntAsFloat(0x7f000000 + ia);
    const float t = IntAsFloat((i << 23) - ia);
    r = r * s;
    r = r * t;

    return r;
}

float FastExp(float x)
{
    // implementation based on: "A more accurate, performance-competitive implementation of expf" by njuffa

    // handle special cases: severe overflow / underflow
    if (x >= 87.0f) // overflow
    {
        return std::numeric_limits<float>::infinity();
    }
    else if (x <= -87.0f) // underflow
    {
        return 0.0f;
    }

    const float t = x * 1.442695041f;
    const float fi = floorf(t);
    const int32 i = (int32)fi;
    const float f = t - fi;

    Bits32 bits;
    bits.f = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f;
    bits.si += (i << 23);
    return bits.f;
}

float Log(float x)
{
    // based on:
    // https://stackoverflow.com/questions/39821367/very-fast-logarithm-natural-log-function-in-c

    NFE_ASSERT(x > 0.0f, "Invalid argument");

    // range reduction
    const int32 e = (FloatAsInt(x) - 0x3f2aaaab) & 0xff800000;
    const float m = IntAsFloat(FloatAsInt(x) - e);
    const float i = 1.19209290e-7f * (float)e;

    const float f = m - 1.0f;
    const float s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    float r = -0.130187988f * f + 0.140889585f;
    float t = -0.121489584f * f + 0.139809534f;
    r = r * s + t;
    r = r * f  -0.166845024f;
    r = r * f + 0.200121149f;
    r = r * f - 0.249996364f;
    r = r * f + 0.333331943f;
    r = r * f - 0.500000000f;
    r = r * s + f;
    r = i * 0.693147182f + r; // log(2)

    return r;
}

float FastLog(float x)
{
    // based on:
    // https://stackoverflow.com/questions/39821367/very-fast-logarithm-natural-log-function-in-c

    // range reduction
    const int32 e = (FloatAsInt(x) - 0x3f2aaaab) & 0xff800000;
    const float m = IntAsFloat(FloatAsInt(x) - e);
    const float i = 1.19209290e-7f * (float)e;

    const float f = m - 1.0f;
    const float s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    float r = 0.230836749f * f - 0.279208571f;
    float t = 0.331826031f * f - 0.498910338f;
    r = r * s + t;
    r = r * s + f;
    r = i * 0.693147182f + r; // log(2)

    return r;
}


} // namespace Math
} // namespace NFE
