/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImagePNG class declaration.
 */

#pragma once

#include "ImageType.hpp"


namespace NFE {
namespace Common {

class ImagePNG : public ImageType
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ImagePNG)

public:
    virtual StringView GetName() const;
    virtual bool Check(InputStream* stream) override;
    virtual bool Load(Image* img, InputStream* stream) override;
    virtual bool Save(Image* img, OutputStream* stream) override;
};

} // namespace Common
} // namespace NFE
