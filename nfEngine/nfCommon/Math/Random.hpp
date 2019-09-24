#pragma once

#include "../nfCommon.hpp"

#include "Float2.hpp"
#include "Float3.hpp"
#include "Vector4.hpp"
#include "Vector2x8.hpp"
#include "VectorInt4.hpp"
#include "VectorInt8.hpp"

namespace NFE {
namespace Math {

// Pseudorandom number generator
class NFE_ALIGN(32) NFCOMMON_API Random
{
public:
    Random();

    // initialize seeds with new values, very slow
    void Reset();

    uint64 GetLong();
    uint32 GetInt();

    // Generate random float with uniform distribution from range [0.0f, 1.0f)
    float GetFloat();
    double GetDouble();

    NFE_FORCE_INLINE const Float2 GetFloat2()
    {
        return GetVector4().ToFloat2();
    }

    NFE_FORCE_INLINE const Float3 GetFloat3()
    {
        return GetVector4().ToFloat3();
    }

    NFE_FORCE_INLINE const Float3 GetFloat4()
    {
        return GetVector4().ToFloat4();
    }

    // Generate random float with uniform distribution from range [-1.0f, 1.0f)
    // faster than "GetFloat()*2.0f-1.0f"
    float GetFloatBipolar();

    // generate random vector of 4 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 4 times
    const Vector4 GetVector4();

    // generate random vector of 4 elements from range [-1.0f, 1.0f)
    const Vector4 GetVector4Bipolar();

    // generate random vector of 8 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 8 times
    const Vector8 GetVector8();

    // generate random vector of 8 elements from range [-1.0f, 1.0f)
    const Vector8 GetVector8Bipolar();

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
    NFE_FORCE_INLINE VectorInt8 GetIntVector8();
    NFE_FORCE_INLINE VectorInt4 GetIntVector4();

#ifdef NFE_USE_AVX2
    VectorInt8 mSeedSimd8[2];
#endif // NFE_USE_AVX2

    VectorInt4 mSeedSimd4[2];

    uint64 mSeed[2];
};

} // namespace Math
} // namespace NFE
