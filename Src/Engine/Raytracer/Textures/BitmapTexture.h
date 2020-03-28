#pragma once

#include "Texture.h"
#include "../../Common/Containers/UniquePtr.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Math/Vec4f.hpp"

namespace NFE {
namespace RT {

class Bitmap;
using BitmapPtr = Common::SharedPtr<Bitmap>;

enum class BitmapTextureFilter : uint8
{
    NearestNeighbor = 0,
    Linear = 1,
    Linear_SmoothStep = 2,
};

// texture wrapper for Bitmap class
class BitmapTexture : public ITexture
{
public:
    NFE_RAYTRACER_API BitmapTexture();
    NFE_RAYTRACER_API BitmapTexture(const BitmapPtr& bitmap);
    ~BitmapTexture();

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec2f u, Math::Vec4f& outCoords, float* outPdf) const override;

    virtual bool MakeSamplable() override;
    virtual bool IsSamplable() const override;

private:
    BitmapPtr mBitmap;
    Common::UniquePtr<Math::Distribution> mImportanceMap;
    BitmapTextureFilter mFilter;
};

} // namespace RT
} // namespace NFE
