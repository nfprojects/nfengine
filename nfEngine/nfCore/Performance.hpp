/**
 * @file   Performance.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance utilities declarations.
 */

#pragma once

#include "Core.hpp"

namespace NFE {
namespace Util {

template <typename T, int N>
class SmoothVar
{
private:
    T mData[N];
    int mValues;

public:
    SmoothVar()
    {
        mValues = 0;
        for (int i = 0; i < N; i++)
            mData[i] = (T)0;
    }

    T& operator= (const T& val)
    {
        mData[0] = val;
        return mData[0];
    }

    T& operator+= (const T& val)
    {
        mData[0] += val;
        return mData[0];
    }

    void Step()
    {
        mValues++;
        if (mValues > N - 1) mValues = N - 1;

        for (int i = mValues; i > 0; i--)
            mData[i] = mData[i - 1];
        mData[0] = 0;
    }

    T Get()
    {
        T sum = (T)0;

        for (int i = 1; i <= mValues; i++)
            sum += mData[i];
        sum /= (T)mValues;

        return sum;
    }
};

//global frame statistics
class FrameStats
{
public:
    FrameStats();

    SmoothVar<double, 20> deltaTime;
    SmoothVar<double, 2> physics;
    SmoothVar<double, 2> shadowsRendering;

    uint32 renderedMeshes;

    void Reset();
};

extern FrameStats g_FrameStats;

} // namespace Utils
} // namespace NFE
