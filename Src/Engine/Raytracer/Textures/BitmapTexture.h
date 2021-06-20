#pragma once

#include "Texture.h"
#include "../../Common/Containers/UniquePtr.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Math/Vec4f.hpp"
#include "../../Common/Reflection/ReflectionEnumMacros.hpp"

namespace NFE {
namespace RT {

class Bitmap;
using BitmapPtr = Common::SharedPtr<Bitmap>;

enum class BitmapTextureFilter : uint8
{
    NearestNeighbor = 0,
    Linear = 1,
    LinearSmoothStep = 2,
    Bicubic = 3,
};

// texture wrapper for Bitmap class
class BitmapTexture : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(BitmapTexture)

public:
    NFE_RAYTRACER_API BitmapTexture();
    NFE_RAYTRACER_API BitmapTexture(const BitmapPtr& bitmap);
    ~BitmapTexture();

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec3f u, Math::Vec4f& outCoords, SampleDistortion distortion, float* outPdf) const override;
    virtual float Pdf(SampleDistortion distortion, const Math::Vec4f& coords) const override;

    virtual bool MakeSamplable(SampleDistortion distortion) override;
    virtual bool IsSamplable(SampleDistortion distortion) const override;

private:
    const Math::Distribution* GetImportanceMap(const SampleDistortion distortion) const;

    BitmapPtr mBitmap;
    Common::UniquePtr<Math::Distribution> mImportanceMap[2];

    BitmapTextureFilter mFilter;
    uint8 mBicubicB;
    uint8 mBicubicC;
};

} // namespace RT
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::RT::BitmapTextureFilter)
