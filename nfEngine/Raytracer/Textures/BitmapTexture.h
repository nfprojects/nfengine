#pragma once

#include "Texture.h"
#include "../../nfCommon/Containers/UniquePtr.hpp"
#include "../../nfCommon/Containers/SharedPtr.hpp"
#include "../../nfCommon/Math/Vector4.hpp"

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
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf) const override;

    virtual bool MakeSamplable() override;
    virtual bool IsSamplable() const override;

private:
    BitmapPtr mBitmap;
    Common::UniquePtr<Math::Distribution> mImportanceMap;
    BitmapTextureFilter mFilter;
};

} // namespace RT
} // namespace NFE
