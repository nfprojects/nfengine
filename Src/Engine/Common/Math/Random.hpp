#pragma once

#include "../nfCommon.hpp"

#include "Vec2f.hpp"
#include "Vec3f.hpp"
#include "Vec4f.hpp"
#include "Vec4i.hpp"
#include "Vec8i.hpp"
#include "Vec16f.hpp"

namespace NFE {
namespace Math {

// Pseudorandom number generator
class NFE_ALIGN(32) NFCOMMON_API Random
{
public:
    Random();

    // initialize seeds with new values, very slow
    void Reset();

    template<typename T>
    const T Get();

    uint64 GetLong();
    uint32 GetInt();

    // Generate random float with uniform distribution from range [0.0f, 1.0f)
    float GetFloat();
    double GetDouble();

    NFE_FORCE_INLINE const Vec2f GetVec2f()
    {
        return GetVec4f().ToVec2f();
    }

    NFE_FORCE_INLINE const Vec3f GetVec3f()
    {
        return GetVec4f().ToVec3f();
    }

    NFE_FORCE_INLINE const Vec4fU GetVec4fU()
    {
        return GetVec4f().ToVec4fU();
    }

    // Generate random float with uniform distribution from range [-1.0f, 1.0f)
    // faster than "GetFloat()*2.0f-1.0f"
    float GetFloatBipolar();

    // generate random vector of 4 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 4 times
    const Vec4f GetVec4f();

    // generate random vector of 4 elements from range [-1.0f, 1.0f)
    const Vec4f GetVec4fBipolar();

    // generate random vector of 8 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 8 times
    const Vec8f GetVec8f();

    // generate random vector of 8 elements from range [-1.0f, 1.0f)
    const Vec8f GetVec8fBipolar();

    /**
     * Fisher-Yates shuffle algorithm.
     *
     * Select @p n random elements in a container.
     * The selected elements are placed at the beginning.
     *
     * For example, executing the function on input:
     *     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
     * with n = 4, may result in:
     *     {8, 6, 4, 7, 0, 1, 2, 3, 5, 9},
     *
     * @param begin,end Iterators pointing at the container beginning and end.
     * @param n Number to elements to select.
     */
    template<class Iter>
    void ShuffleContainer(Iter begin, const Iter& end, size_t n)
    {
        size_t left = std::distance(begin, end);

        if (n > left)
        {
            n = left;
        }

        while (n--)
        {
            Iter r = begin;
            std::advance(r, static_cast<size_t>(GetLong()) % left);
            std::swap(*begin, *r);
            ++begin;
            --left;
        }
    }

private:
    NFE_FORCE_INLINE Vec8i GetVec8i();
    NFE_FORCE_INLINE Vec4i GetVec4i();

#ifdef NFE_USE_AVX2
    Vec8i mSeedSimd8[2];
#endif // NFE_USE_AVX2

    Vec4i mSeedSimd4[2];

    uint64 mSeed[2];
};


template<>
NFE_FORCE_INLINE const bool Random::Get()
{
    return GetInt() & 1;
}

template<>
NFE_FORCE_INLINE const uint8 Random::Get()
{
    return static_cast<uint8>(GetInt());
}

template<>
NFE_FORCE_INLINE const int8 Random::Get()
{
    return static_cast<int8>(GetInt());
}

template<>
NFE_FORCE_INLINE const uint16 Random::Get()
{
    return static_cast<uint16>(GetInt());
}

template<>
NFE_FORCE_INLINE const int16 Random::Get()
{
    return static_cast<int16>(GetInt());
}

template<>
NFE_FORCE_INLINE const uint32 Random::Get()
{
    return GetInt();
}

template<>
NFE_FORCE_INLINE const int32 Random::Get()
{
    return static_cast<int32>(GetInt());
}

template<>
NFE_FORCE_INLINE const uint64 Random::Get()
{
    return GetLong();
}

template<>
NFE_FORCE_INLINE const int64 Random::Get()
{
    return static_cast<int32>(GetLong());
}

template<>
NFE_FORCE_INLINE const float Random::Get()
{
    return GetFloat();
}

template<>
NFE_FORCE_INLINE const double Random::Get()
{
    return GetDouble();
}

template<>
NFE_FORCE_INLINE const Vec4f Random::Get()
{
    return GetVec4f();
}

template<>
NFE_FORCE_INLINE const Vec8f Random::Get()
{
    return GetVec8f();
}

template<>
NFE_FORCE_INLINE const Vec16f Random::Get()
{
    return Vec16f{ GetVec8f(), GetVec8f() };
}

} // namespace Math
} // namespace NFE
