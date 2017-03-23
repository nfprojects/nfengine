/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageType interface declaration
 */

#pragma once

#include "ImageFormat.hpp"

#include <vector>


namespace NFE {
namespace Common {

using ImageTypePtr = std::unique_ptr<ImageType>;
using ImageTypeMap = std::map<String, ImageTypePtr>;

class NFCOMMON_API ImageType
{
public:
    ImageType() {}
    ~ImageType() {}

    virtual bool Check(InputStream*) = 0;
    virtual bool Load(Image*, InputStream*) = 0;
    virtual bool Save(Image*, OutputStream*) = 0;

    /**
     * Register an image type to make it available for saving/loading.
     *
     * @param name      Image type name.
     * @param imageType Image type object - must implement ImageType class.
     *
     * @return True, if new ImageType with @name was inserted. False if @name is already in use.
     */
    static bool RegisterImageType(const String& name, ImageTypePtr imageType);

    /**
     * Get pointer to an already registered ImageType.
     *
     * @param name     Image type name.
     *
     * @return Pointer to the ImageType if registered, otherwise nullptr.
     */
    static ImageType* GetImageType(const String& name);

    /**
     * Get list of the registered ImageTypes.
     */
    static std::vector<String> ListImageTypes();

protected:
    std::vector<Mipmap>* GetMipmaps(Image* img);
};

} // namespace Common
} // namespace NFE
