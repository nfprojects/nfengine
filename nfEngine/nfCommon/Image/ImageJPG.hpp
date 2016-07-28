/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageJPG class declaration.
 */

#pragma once
#include "Image.hpp"
#include "ImageType.hpp"

#include <vector>

namespace NFE {
namespace Common {

class ImageJPG : public ImageType
{
public:
    ImageJPG() {};
    ~ImageJPG() {};

    bool Check(InputStream* stream) override;
    bool Load(Image* img, InputStream* stream) override;
    bool Save(Image* img, OutputStream* stream) override;
};

} // namespace Common
} // namespace NFE
