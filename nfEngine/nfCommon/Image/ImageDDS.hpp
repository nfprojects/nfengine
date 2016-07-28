/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageDDS class declaration.
 */

#pragma once
#include "Image.hpp"
#include "ImageType.hpp"

#include <vector>

namespace NFE {
namespace Common {

class ImageDDS : public ImageType
{
public:
    ImageDDS() {};
    ~ImageDDS() {};

    bool Check(InputStream* stream) override;
    bool Load(Image* img, InputStream* stream) override;
    bool Save(Image* img, OutputStream* stream) override;
};

} // namespace Common
} // namespace NFE
