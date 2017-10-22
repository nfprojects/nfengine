/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageBMP class declaration.
 */

#pragma once
#include "ImageType.hpp"

namespace NFE {
namespace Common {

class Image;

class ImageBMP : public ImageType
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ImageBMP)

public:
    virtual StringView GetName() const override;
    virtual bool Check(InputStream* stream) override;
    virtual bool Load(Image* img, InputStream* stream) override;
    virtual bool Save(Image* img, OutputStream* stream) override;
};

} // namespace Common
} // namespace NFE
