/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageType interface class declarations
 */

#include "../PCH.hpp"
#include "ImageType.hpp"
#include "Image.hpp"

namespace NFE {
namespace Common {

void ImageType::SetWidth(Image* img, int w)
{
    img->mWidth = w;
}

void ImageType::SetHeight(Image* img, int h)
{
    img->mHeight = h;
}

void ImageType::SetFormat(Image* img, ImageFormat f)
{
    img->mFormat = f;
}

std::vector<Mipmap>* ImageType::GetMipmaps(Image* img)
{
    return &img->mMipmaps;
}

} // namespace Common
} // namespace NFE
