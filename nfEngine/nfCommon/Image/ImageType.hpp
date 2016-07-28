/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageType interface declaration
 */

#pragma once
#include "ImageFormat.hpp"
#include "../InputStream.hpp"
#include "../OutputStream.hpp"

#include <vector>

namespace NFE {
namespace Common {

class Image;
class Mipmap;

class ImageType
{
public:
    ImageType() {};
    ~ImageType() {};

    virtual bool Check(InputStream*) = 0;
    virtual bool Load(Image*, InputStream*) = 0;
    virtual bool Save(Image*, OutputStream*) = 0;

protected:
    void SetWidth(Image* img, int w);
    void SetHeight(Image* img, int h);
    void SetFormat(Image* img, ImageFormat f);
    std::vector<Mipmap>* GetMipmaps(Image* img);
};

} // namespace Common
} // namespace NFE