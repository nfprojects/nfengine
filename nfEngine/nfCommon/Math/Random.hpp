/**
 * @file   Random.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Pseudorandom number generator declaration.
 */

#pragma once

namespace NFE {
namespace Math {

/*
    Pseudorandom number generator
*/
class NFCOMMON_API Random
{
private:
    unsigned __int64 mSeed;

    // XOR-shift algorithm
    __forceinline void Shuffle()
    {
        mSeed ^= (mSeed << 21);
        mSeed ^= (mSeed >> 35);
        mSeed ^= (mSeed << 4);
    }

public:
    Random();
    Random(__int64 seed);

    unsigned __int64 GetLong();
    unsigned int GetInt();

    //Generate random float with uniform distribution from range (0.0f, 1.0f]
    float GetFloat();
    double GetDouble();

    //Generate random float with uniform distribution from range [-1.0f, 1.0f)
    //faster than "GetFloat()*2.0f-1.0f"
    float GetFloatBipolar();

    //generate uniformly distributed float vectors
    Float2 GetFloat2();
    Float3 GetFloat3();
    Float4 GetFloat4();

    //Generate random float (vector) with gaussian distribution. (SLOW)
    float GetFloatNormal();
    Float2 GetFloatNormal2();
    Float3 GetFloatNormal3();
    Float4 GetFloatNormal4();
};

} // namespace Math
} // namespace NFE
