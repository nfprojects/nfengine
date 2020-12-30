#pragma once

#include "BitmapTexture.h"

namespace NFE {
namespace RT {

// 3D texture wrapper for Bitmap class
class BitmapTexture3D : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(BitmapTexture3D)

public:
    NFE_RAYTRACER_API BitmapTexture3D();
    NFE_RAYTRACER_API BitmapTexture3D(const BitmapPtr& bitmap);
    ~BitmapTexture3D();

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec2f u, Math::Vec4f& outCoords, float* outPdf) const override;

    virtual bool MakeSamplable() override;
    virtual bool IsSamplable() const override;

private:
    BitmapPtr mBitmap;
    BitmapTextureFilter mFilter;
};

} // namespace RT
} // namespace NFE
