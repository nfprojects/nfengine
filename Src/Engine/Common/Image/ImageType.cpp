/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageType interface class declarations
 */

#include "PCH.hpp"
#include "ImageType.hpp"
#include "Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Common::ImageType)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace Common {


DynArray<ImageTypePtr> ImageType::GetTypes()
{
    DynArray<ImageTypePtr> imageTypes;

    // construct image type objects via RTTI
    DynArray<const RTTI::ClassType*> types;
    RTTI::GetType<ImageType>()->ListSubtypes(types);
    for (const RTTI::ClassType* type : types)
    {
        if (type->IsConstructible())
        {
            ImageTypePtr imageType(type->CreateObject<ImageType>());
            imageTypes.PushBack(std::move(imageType));
        }
    }

    return imageTypes;
}

ImageTypePtr ImageType::GetImageType(const StringView name)
{
    DynArray<ImageTypePtr> types = GetTypes();
    for (ImageTypePtr& type : types)
    {
        if (type->GetName() == name)
        {
            return std::move(type);
        }
    }

    return nullptr;
}

} // namespace Common
} // namespace NFE
