#pragma once

#include "BitmapTexture.h"

namespace NFE {
namespace RT {

// 3D texture wrapper for Bitmap class
class BitmapTexture3D : public ITexture
{
public:
    NFE_RAYTRACER_API BitmapTexture3D();
    NFE_RAYTRACER_API BitmapTexture3D(const BitmapPtr& bitmap);
    ~BitmapTexture3D();

    virtual const char* GetName() const override;
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf) const override;

    virtual bool MakeSamplable() override;
    virtual bool IsSamplable() const override;

private:
    BitmapPtr mBitmap;
    BitmapTextureFilter mFilter;
};

} // namespace RT
} // namespace NFE
