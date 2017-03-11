/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageType interface class declarations
 */

#include "PCH.hpp"
#include "ImageType.hpp"
#include "Image.hpp"


namespace NFE {
namespace Common {

bool ImageType::RegisterImageType(const std::string& name, ImageTypePtr imageType)
{
    return Image::mImageTypes().insert(std::make_pair(name, std::move(imageType))).second;
}

ImageType* ImageType::GetImageType(const std::string& name)
{
    ImageTypeMap::const_iterator imageType = Image::mImageTypes().find(name);

    if (imageType == Image::mImageTypes().cend())
        return nullptr;

    return imageType->second.get();
}

std::vector<std::string> ImageType::ListImageTypes()
{
    std::vector<std::string> vect;
    vect.reserve(Image::mImageTypes().size());

    for (const auto& i : Image::mImageTypes())
        vect.push_back(i.first);

    return vect;
}

std::vector<Mipmap>* ImageType::GetMipmaps(Image* img)
{
    return &img->mMipmaps;
}

} // namespace Common
} // namespace NFE
