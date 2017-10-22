/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageType interface declaration
 */

#pragma once

#include "ImageFormat.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/StringView.hpp"
#include "../Containers/HashMap.hpp"
#include "../Reflection/ReflectionClassMacros.hpp"


namespace NFE {
namespace Common {

using ImageTypePtr = UniquePtr<ImageType>;
using ImageTypeMap = HashMap<StringView, ImageTypePtr>;

class NFCOMMON_API ImageType
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ImageType)

public:
    virtual ~ImageType() {}

    virtual StringView GetName() const = 0;
    virtual bool Check(InputStream*) = 0;
    virtual bool Load(Image*, InputStream*) = 0;
    virtual bool Save(Image*, OutputStream*) = 0;

    /**
     * List image types.
     */
    static DynArray<ImageTypePtr> GetTypes();

    /**
     * Get pointer to an already registered ImageType.
     *
     * @param name     Image type name.
     *
     * @return Pointer to the ImageType if registered, otherwise nullptr.
     */
    static ImageTypePtr GetImageType(const StringView name);
};

} // namespace Common
} // namespace NFE
