/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Pseudorandom number generator declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "Math.hpp"

#include <iterator>


namespace NFE {
namespace Math {

/**
 * Pseudorandom number generator
 */
class NFCOMMON_API Random
{
private:
    uint64 mSeed;

    // XOR-shift algorithm
    NFE_INLINE void Shuffle()
    {
        mSeed ^= (mSeed << 21);
        mSeed ^= (mSeed >> 35);
        mSeed ^= (mSeed << 4);
    }

public:
    Random();
    Random(uint64 seed);

    uint64 GetLong();
    int GetInt();

    // Generate random float with uniform distribution from range (0.0f, 1.0f]
    float GetFloat();
    double GetDouble();

    // Generate random float with uniform distribution from range [-1.0f, 1.0f)
    // faster than "GetFloat()*2.0f-1.0f"
    float GetFloatBipolar();

    // generate uniformly distributed float vectors
    Float2 GetFloat2();
    Float3 GetFloat3();
    Float4 GetFloat4();

    // Generate random float (vector) with Gaussian distribution. (SLOW)
    float GetFloatNormal();
    Float2 GetFloatNormal2();
    Float3 GetFloatNormal3();
    Float4 GetFloatNormal4();

    /**
     * Generate random point on a circle (uniform distribution).
     * @note This is slow.
     */
    Float2 GetPointInsideCircle();

    /**
     * Generate random point on a sphere (uniform distribution).
     * @note This is slow.
     */
    Float3 GetPointOnSphere();

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
            n = left;

        while (n--)
        {
            Iter r = begin;
            std::advance(r, static_cast<size_t>(GetLong()) % left);
            std::swap(*begin, *r);
            ++begin;
            --left;
        }
    }

};

} // namespace Math
} // namespace NFE
