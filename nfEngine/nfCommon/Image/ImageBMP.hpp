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
public:
    ImageBMP() {};
    ~ImageBMP() {};

    bool Check(InputStream* stream) override;
    bool Load(Image* img, InputStream* stream) override;
    bool Save(Image* img, OutputStream* stream) override;
};

} // namespace Common
} // namespace NFE
