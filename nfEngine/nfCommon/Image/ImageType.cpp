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

bool ImageType::RegisterImageType(const StringView name, ImageTypePtr imageType)
{
    return Image::mImageTypes().Insert(name, std::move(imageType)).iterator != Image::mImageTypes().End();
}

ImageType* ImageType::GetImageType(const StringView name)
{
    const ImageTypeMap::ConstIterator imageType = Image::mImageTypes().Find(name);

    if (imageType == Image::mImageTypes().cend())
        return nullptr;

    return imageType->second.Get();
}

DynArray<StringView> ImageType::ListImageTypes()
{
    DynArray<StringView> array;
    array.Reserve(Image::mImageTypes().Size());

    for (const auto& i : Image::mImageTypes())
        array.PushBack(i.first);

    return array;
}

DynArray<Mipmap>* ImageType::GetMipmaps(Image* img)
{
    return &img->mMipmaps;
}

} // namespace Common
} // namespace NFE
