#include "PCH.hpp"
#include "Transcendental.hpp"
#include "Vec16i.hpp"

namespace NFE {
namespace Math {

namespace {

NFE_FORCE_INLINE int32 FloatAsInt(const float f)
{
    Common::FundamentalTypesUnion bits;
    bits.f = f;
    return bits.i32;
}

NFE_FORCE_INLINE float IntAsFloat(const int32 i)
{
    Common::FundamentalTypesUnion bits;
    bits.i32 = i;
    return bits.f;
}

} // namespace

namespace sinCoeffs
{
    static const float c0 =  9.9999970197e-01f;
    static const float c1 = -1.6666577756e-01f;
    static const float c2 =  8.3325579762e-03f;
    static const float c3 = -1.9812576647e-04f;
    static const float c4 =  2.7040521217e-06f;
    static const float c5 = -2.0532988642e-08f;
}

float Sin(float x)
{
    using namespace sinCoeffs;

    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const int32 i = static_cast<int32>(x * (1.0f / NFE_MATH_PI));
    x -= static_cast<float>(i) * NFE_MATH_PI;

    const float x2 = x * x;

    float y = x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));

    return (i & 1) ? -y : y;
}

const Vec4f Sin(const Vec4f& a)
{
    using namespace sinCoeffs;

    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const Vec4i i = Vec4i::Convert(a * (1.0f / NFE_MATH_PI));
    const Vec4f x = Vec4f::NegMulAndAdd(i.ConvertToVec4f(), NFE_MATH_PI, a);

    const Vec4f x2 = x * x;

    Vec4f y = Vec4f::MulAndAdd(Vec4f(c5), x2, Vec4f(c4));
    y = Vec4f::MulAndAdd(y, x2, Vec4f(c3));
    y = Vec4f::MulAndAdd(y, x2, Vec4f(c2));
    y = Vec4f::MulAndAdd(y, x2, Vec4f(c1));
    y = Vec4f::MulAndAdd(y, x2, Vec4f(c0));
    y *= x;

    // equivalent of: (i & 1) ? -y : y;
    return y ^ (i << 31).AsVec4f();
}

const Vec8f Sin(const Vec8f& a)
{
#ifdef NFE_USE_AVX2
    using namespace sinCoeffs;

    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const Vec8i i = Vec8i::Convert(a * (1.0f / NFE_MATH_PI));
    const Vec8f x = Vec8f::NegMulAndAdd(i.ConvertToVec8f(), NFE_MATH_PI, a);

    const Vec8f x2 = x * x;

    Vec8f y = Vec8f::MulAndAdd(Vec8f(c5), x2, Vec8f(c4));
    y = Vec8f::MulAndAdd(y, x2, Vec8f(c3));
    y = Vec8f::MulAndAdd(y, x2, Vec8f(c2));
    y = Vec8f::MulAndAdd(y, x2, Vec8f(c1));
    y = Vec8f::MulAndAdd(y, x2, Vec8f(c0));
    y *= x;

    // equivalent of: (i & 1) ? -y : y;
    return y ^ (i << 31).AsVec8f();
#else
    return Vec8f{sinf(a[0]), sinf(a[1]), sinf(a[2]), sinf(a[3]), sinf(a[4]), sinf(a[5]), sinf(a[6]), sinf(a[7])};
#endif // NFE_USE_AVX2
}

const Vec16f Sin(const Vec16f& a)
{
    using namespace sinCoeffs;

    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const Vec16i i = Vec16i::Convert(a * (1.0f / NFE_MATH_PI));
    const Vec16f x = Vec16f::NegMulAndAdd(i.ConvertToVec16f(), NFE_MATH_PI, a);

    const Vec16f x2 = x * x;

    Vec16f y = Vec16f::MulAndAdd(Vec16f(c5), x2, Vec16f(c4));
    y = Vec16f::MulAndAdd(y, x2, Vec16f(c3));
    y = Vec16f::MulAndAdd(y, x2, Vec16f(c2));
    y = Vec16f::MulAndAdd(y, x2, Vec16f(c1));
    y = Vec16f::MulAndAdd(y, x2, Vec16f(c0));
    y *= x;

    // equivalent of: (i & 1) ? -y : y;
    return y ^ (i << 31).AsVec16f();
}

// calculates acos(|x|)
static NFE_INLINE float ACosAbs(float x)
{
    // based on DirectXMath implementation:
    // https://github.com/Microsoft/DirectXMath/blob/master/Inc/DirectXMathMisc.inl

    x = fabsf(x);
    float root = sqrtf(1.0f - x);

    const float c0 = 1.5707963050f;
    const float c1 = -0.2145988016f;
    const float c2 = 0.0889789874f;
    const float c3 = -0.0501743046f;
    const float c4 = 0.0308918810f;
    const float c5 = -0.0170881256f;
    const float c6 = 0.0066700901f;
    const float c7 = -0.0012624911f;

    return root * (c0 + x * (c1 + x * (c2 + x * (c3 + x * (c4 + x * (c5 + x * (c6 + x * c7)))))));
}

float ACos(float x)
{
    NFE_ASSERT(x >= -1.0f && x <= 1.0f, "Invalid argument");

    bool nonnegative = (x >= 0.0f);
    const float acosAbs = ACosAbs(x);

    // acos(x) = pi - acos(-x) when x < 0
    return nonnegative ? acosAbs : Constants::pi<float> -acosAbs;
}

float ASin(float x)
{
    NFE_ASSERT(x >= -1.0f && x <= 1.0f, "Invalid argument");

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
    float y = 2.78569828e-3f;
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

float FastACos(float x)
{
    // based on:
    // https://stackoverflow.com/a/26030435/10061517

    float negate = float(x < 0);
    x = fabsf(x);
    float ret = -0.0187293f;
    ret = ret * x + 0.0742610f;
    ret = ret * x - 0.2121144f;
    ret = ret * x + 1.5707288f;
    ret = ret * sqrtf(1.0f - x);
    ret = ret - 2.0f * negate * ret;
    return negate * 3.14159265358979f + ret;
}

float FastExp2(float a)
{
    // implementation based on: "A more accurate, performance-competitive implementation of expf" by njuffa

    // handle special cases: severe overflow / underflow
    if (a >= 128.0f) // overflow
    {
        return std::numeric_limits<float>::infinity();
    }
    else if (a <= -125.0f) // underflow
    {
        return 0.0f;
    }

    const float fi = floorf(a);
    const int32 i = (int32)fi;
    const float f = a - fi;

    Common::FundamentalTypesUnion bits;
    bits.f = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f;
    bits.i32 += (i << 23);
    return bits.f;
}

const Vec4f FastExp2(const Vec4f& a)
{
    const Vec4f fi = Vec4f::Floor(a);
    const Vec4i i = Vec4i::Convert(fi);
    const Vec4f f = a - fi;

    Vec4f y = Vec4f::MulAndAdd(f, Vec4f(0.3371894346f), Vec4f(0.657636276f));
    y = Vec4f::MulAndAdd(f, y, Vec4f(1.00172476f));

    Vec4i yi = Vec4i::Cast(y);
    yi += (i << 23);
    y = yi.AsVec4f();

    // handle overflow
    y = Vec4f::Select(y, Vec4f::Zero(), -a >= Vec4f(125.0f));
    y = Vec4f::Select(y, VECTOR_INF, a >= Vec4f(128.0f));

    return y;
}

const Vec8f FastExp2(const Vec8f& a)
{
    const Vec8f fi = Vec8f::Floor(a);
    const Vec8i i = Vec8i::Convert(fi);
    const Vec8f f = a - fi;

    Vec8f y = Vec8f::MulAndAdd(f, Vec8f(0.3371894346f), Vec8f(0.657636276f));
    y = Vec8f::MulAndAdd(f, y, Vec8f(1.00172476f));

    Vec8i yi = Vec8i::Cast(y);
    yi += (i << 23);
    y = yi.AsVec8f();

    // handle overflow
    y = Vec8f::Select(y, Vec8f::Zero(), -a >= Vec8f(125.0f));
    y = Vec8f::Select(y, VECTOR8_INF, a >= Vec8f(128.0f));

    return y;
}

float Log(float x)
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
    float r = -0.130187988f * f + 0.140889585f;
    float t = -0.121489584f * f + 0.139809534f;
    r = r * s + t;
    r = r * f - 0.166845024f;
    r = r * f + 0.200121149f;
    r = r * f - 0.249996364f;
    r = r * f + 0.333331943f;
    r = r * f - 0.500000000f;
    r = r * s + f;
    r = i * 0.693147182f + r; // log(2)
    return r;
}

float FastLog2(float x)
{
    // based on:
    // https://stackoverflow.com/questions/9411823/fast-log2float-x-implementation-c/9411984#9411984

    Common::FundamentalTypesUnion u;
    u.f = x;
    float result = (float)(((u.i32 >> 23) & 255) - 128);
    u.i32 &= ~(255 << 23);
    u.i32 += 127 << 23;
    result += ((-0.33333333f) * u.f + 2.0f) * u.f - 0.66666666f;
    return result;
}

const Vec4f FastLog2(const Vec4f& a)
{
    // based on:
    // https://stackoverflow.com/questions/9411823/fast-log2float-x-implementation-c/9411984#9411984

    Vec4i i = Vec4i::Cast(a);
    Vec4f r = (((i >> 23)& Vec4i(255)) - Vec4i(128)).ConvertToVec4f();
    i &= Vec4i(~(255 << 23));
    i += 127 << 23;
    Vec4f f = i.AsVec4f();
    r += (-0.33333333f * f + Vec4f(2.0f)) * f - Vec4f(0.66666666f);
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

const Vec4f FastLog(const Vec4f& a)
{
    // range reduction
    const Vec4i e = (Vec4i::Cast(a) - Vec4i(0x3f2aaaab)) & Vec4i(0xff800000);
    const Vec4f m = (Vec4i::Cast(a) - e).AsVec4f();
    const Vec4f i = e.ConvertToVec4f() * 1.19209290e-7f;

    const Vec4f f = m - Vec4f(1.0f);
    const Vec4f s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    Vec4f r = Vec4f::MulAndAdd(f, Vec4f(0.230836749f), Vec4f(-0.279208571f));
    Vec4f t = Vec4f::MulAndAdd(f, Vec4f(0.331826031f), Vec4f(-0.498910338f));
    r = Vec4f::MulAndAdd(r, s, t);
    r = Vec4f::MulAndAdd(r, s, f);
    r = Vec4f::MulAndAdd(i, Vec4f(0.693147182f), r); // log(2)
    return r;
}

float FastATan2(const float y, const float x)
{
    // https://stackoverflow.com/questions/46210708/atan2-approximation-with-11bits-in-mantissa-on-x86with-sse2-and-armwith-vfpv4

    const float ax = Math::Abs(x);
    const float ay = Math::Abs(y);
    const float mx = Math::Max(ay, ax);
    const float mn = Math::Min(ay, ax);
    const float a = mn / mx;

    // Minimax polynomial approximation to atan(a) on [0,1]
    const float s = a * a;
    const float c = s * a;
    const float q = s * s;
    const float t = -0.094097948f * q - 0.33213072f;
    float r = (0.024840285f * q + 0.18681418f);
    r = r * s + t;
    r = r * c + a;

    // Map to full circle
    if (ay > ax) r = 1.57079637f - r;
    if (x < 0.0f) r = NFE_MATH_PI - r;
    if (y < 0.0f) r = -r;
    return r;
}

} // namespace Math
} // namespace NFE
