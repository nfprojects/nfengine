#pragma once

#include "Bitmap.h"

namespace NFE {
namespace RT {

/**
 * Class representing 2D bitmap.
 */
class BitmapUtils
{
public:
    struct GaussianBlurParams
    {
        float sigma;
        uint32 numPasses;
    };

    static bool GaussianBlur(Bitmap& targetBitmap, const Bitmap& sourceBitmap, const GaussianBlurParams params, Common::TaskBuilder& taskBuilder);
};


} // namespace RT
} // namespace NFE
