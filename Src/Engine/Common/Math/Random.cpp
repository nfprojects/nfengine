#include "PCH.hpp"
#include "Random.hpp"
#include "Transcendental.hpp"
#include "../Utils/Entropy.hpp"

namespace NFE {
namespace Math {

static NFE_FORCE_INLINE uint64 Rotl64(const uint64 x, const int k)
{
    return (x << k) | (x >> (64 - k));
}

Random::Random()
{
    Reset();
}

void Random::Reset()
{
    Common::Entropy entropy;
    
    for (uint32 i = 0; i < 2; ++i)
    {
        mSeed[i] = ((uint64)entropy.GetInt() << 32) | (uint64)entropy.GetInt();
        mSeedSimd4[i] = Vec4i(entropy.GetInt(), entropy.GetInt(), entropy.GetInt(), entropy.GetInt());
#ifdef NFE_USE_AVX2
        mSeedSimd8[i] = Vec8i(entropy.GetInt(), entropy.GetInt(), entropy.GetInt(), entropy.GetInt(), entropy.GetInt(), entropy.GetInt(), entropy.GetInt(), entropy.GetInt());
#endif // NFE_USE_AVX2
    }
}

uint64 Random::GetLong()
{
    // xoroshiro128+ algorithm
    // http://xoshiro.di.unimi.it/xoroshiro128plus.c

    const uint64 s0 = mSeed[0];
    uint64 s1 = mSeed[1];
    const uint64 result = s0 + s1;

    s1 ^= s0;
    mSeed[0] = Rotl64(s0, 24) ^ s1 ^ (s1 << 16);
    mSeed[1] = Rotl64(s1, 37);

    return result;
}

uint32 Random::GetInt()
{
    return static_cast<uint32>(GetLong());
}

float Random::GetFloat()
{
    Common::FundamentalTypesUnion myrand;
    myrand.u32 = (GetInt() & 0x007fffffu) | 0x3f800000u;
    return myrand.f - 1.0f;
}

double Random::GetDouble()
{
    return static_cast<double>(GetLong()) / static_cast<double>(std::numeric_limits<uint64>::max());
}

float Random::GetFloatBipolar()
{
    Common::FundamentalTypesUnion myrand;
    myrand.u32 = (GetInt() & 0x007fffff) | 0x40000000;
    return myrand.f - 3.0f;
}

Vec4i Random::GetVec4i()
{
    // NOTE: xoroshiro128+ is faster when using general purpose registers, because there's
    // no rotate left/right instruction in SSE2 (it's only in AVX512)

    // xorshift128+ algorithm
    const Vec4i s0 = mSeedSimd4[1];
    Vec4i s1 = mSeedSimd4[0];

    // TODO introduce Vector2ul
#ifdef NFE_USE_SSE
    Vec4i v = _mm_add_epi64(s0, s1);
    s1 = _mm_slli_epi64(s1, 23);
    const Vec4i t0 = _mm_srli_epi64(s0, 5);
    const Vec4i t1 = _mm_srli_epi64(s1, 18);
#else
    Vec4i v;
    v.i64[0] = s0.i64[0] + s1.i64[0];
    v.i64[1] = s0.i64[1] + s1.i64[1];
    s1.i64[0] <<= 23;
    s1.i64[1] <<= 23;
    Vec4i t0, t1;
    t0.i64[0] = s0.i64[0] >> 5;
    t0.i64[1] = s0.i64[1] >> 5;
    t1.i64[0] = s1.i64[0] >> 5;
    t1.i64[1] = s1.i64[1] >> 5;
#endif

    mSeedSimd4[0] = s0;
    mSeedSimd4[1] = (s0 ^ s1) ^ (t0 ^ t1);
    return v;
}

const Vec4f Random::GetVec4f()
{
    Vec4i v = GetVec4i();

    // setup float mask
    v &= Vec4i(0x007fffffu);
    v |= Vec4i(0x3f800000u);

    // convert to float and go from [1, 2) to [0, 1) range
    return v.AsVec4f() - VECTOR_ONE;
}

const Vec4f Random::GetVec4fBipolar()
{
    Vec4i v = GetVec4i();

    // setup float mask
    v &= Vec4i(0x007fffffu);
    v |= Vec4i(0x40000000u);

    // convert to float and go from [2, 4) to [-1, 1) range
    return v.AsVec4f() - Vec4f(3.0f);
}

#ifdef NFE_USE_AVX2

Vec8i Random::GetVec8i()
{
    // NOTE: xoroshiro128+ is faster when using general purpose registers, because there's
    // no rotate left/right instruction in AVX2 (it's only in AVX512)

    // xorshift128+ algorithm
    const Vec8i s0 = mSeedSimd8[1];
    Vec8i s1 = mSeedSimd8[0];
    Vec8i v = _mm256_add_epi64(s0, s1);
    s1 = _mm256_slli_epi64(s1, 23);
    const Vec8i t0 = _mm256_srli_epi64(s0, 5);
    const Vec8i t1 = _mm256_srli_epi64(s1, 18);
    mSeedSimd8[0] = s0;
    mSeedSimd8[1] = (s0 ^ s1) ^ (t0 ^ t1);

    return v;
}

#else

Vec8i Random::GetVec8i()
{
    return Vec8i{ GetVec4i(), GetVec4i() };
}

#endif // NFE_USE_AVX2

const Vec8f Random::GetVec8f()
{
    Vec8i v = GetVec8i();

    // setup float mask
    v &= Vec8i(0x007fffffu);
    v |= Vec8i(0x3f800000u);

    // convert to float and go from [1, 2) to [0, 1) range
    return v.AsVec8f() - VECTOR8_ONE;
}

const Vec8f Random::GetVec8fBipolar()
{
    Vec8i v = GetVec8i();

    // setup float mask
    v &= Vec8i(0x007fffffu);
    v |= Vec8i(0x40000000u);

    // convert to float and go from [1, 2) to [0, 1) range
    return v.AsVec8f() - Vec8f(3.0f);
}

} // namespace Math
} // namespace NFE
