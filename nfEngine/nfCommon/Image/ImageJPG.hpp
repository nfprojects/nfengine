/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageJPG class declaration.
 */

#pragma once

#include "ImageType.hpp"


namespace NFE {
namespace Common {

class ImageJPG : public ImageType
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ImageJPG)

public:
    virtual StringView GetName() const;
    virtual bool Check(InputStream* stream) override;
    virtual bool Load(Image* img, InputStream* stream) override;
    virtual bool Save(Image* img, OutputStream* stream) override;
};

} // namespace Common
} // namespace NFE
