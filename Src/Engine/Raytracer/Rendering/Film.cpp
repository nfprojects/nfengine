#include "PCH.h"
#include "Film.h"
#include "../Utils/Bitmap.h"
#include "../Common/Math/Random.hpp"
#include "../Common/Math/Vec4fLoad.hpp"

namespace NFE {
namespace RT {

using namespace Math;

Film::Film()
    : mFilmSize(Vec4f::Zero())
    , mSum(nullptr)
    , mSecondarySum(nullptr)
    , mWidth(0)
    , mHeight(0)
{}

Film::Film(Bitmap& sum, Bitmap* secondarySum)
    : mFilmSize((float)sum.GetWidth(), (float)sum.GetHeight())
    , mSum(&sum)
    , mSecondarySum(secondarySum) 
    , mWidth(sum.GetWidth())
    , mHeight(sum.GetHeight())
{
    if (mSecondarySum)
    {
        NFE_ASSERT(mSecondarySum->GetWidth() == mWidth);
        NFE_ASSERT(mSecondarySum->GetHeight() == mHeight);
    }
}

NFE_FORCE_INLINE static void AccumulateToFloat3(Vec3f& target, const Vec4f& value)
{
    const Vec4f original = Vec4f_Load_Vec3f_Unsafe(target);
    target = (original + value).ToVec3f();
}

void Film::AccumulateColor(const uint32 x, const uint32 y, const Vec4f& sampleColor)
{
    if (!mSum)
    {
        return;
    }

    Vec3f* sumPixel = &(mSum->GetPixelRef<Vec3f>(x, y));
    Vec3f* secondarySumPixel = mSecondarySum ? &(mSecondarySum->GetPixelRef<Vec3f>(x, y)) : nullptr;

    LockPixel(x, y);
    {
        AccumulateToFloat3(*sumPixel, sampleColor);

        if (secondarySumPixel)
        {
            AccumulateToFloat3(*secondarySumPixel, sampleColor);
        }
    }
    UnlockPixel(x, y);
}

NFE_FORCE_NOINLINE
void Film::AccumulateColor(const Vec4f& pos, const Vec4f& sampleColor, Random& randomGenerator)
{
    if (!mSum)
    {
        return;
    }

    const Vec4f filmCoords = pos * mFilmSize + Vec4f(0.0f, 0.5f);
    Vec4i intFilmCoords = Vec4i::Convert(filmCoords);

    // apply jitter to simulate box filter
    // Note: could just splat to 4 nearest pixels, but may be slower
    {
        const Vec4f coordFraction = filmCoords - intFilmCoords.ConvertToVec4f();
        const Vec4f u = randomGenerator.GetVec4f();

        intFilmCoords = Vec4i::Select(intFilmCoords, intFilmCoords + 1, u < coordFraction);

        //if (u.x < coordFraction.x)
        //{
        //    intFilmCoords.x++;
        //}
        //if (u.y < coordFraction.y)
        //{
        //    intFilmCoords.y++;
        //}
    }

    const int32 x = intFilmCoords.x;
    const int32 y = int32(mHeight - 1) - int32(filmCoords.y);

    if (uint32(x) < mWidth && uint32(y) < mHeight)
    {
        AccumulateColor(x, y, sampleColor);
    }
}

} // namespace RT
} // namespace NFE
