/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Image class declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Containers/DynArray.hpp"
#include "Mipmap.hpp"
#include "Image.hpp"
#include "../Containers/UniquePtr.hpp"
#include "ImageFormat.hpp"


namespace NFE {
namespace Common {
namespace ImageUtils {
    /**
     * Generate mipmaps.
     * @param num Number of mipmaps to generate. Leave this argument default to generate a full set
     * @return true on success
     */
    NFE_API_EXPORT bool GenerateMipmaps(Image* img, MipmapFilter filterType, uint32 num = 0xFFFFFFFF);

    /**
     * Change pixel format. Currently converting from and to BC4-BC7 formats is unsupported.
     * @param destFormat Destination image format
     * @return true on success
     */
    NFE_API_EXPORT bool Convert(Image* src, ImageFormat destFormat);

    /**
     * Load image from a data stream. Supported file formats: BMP, JPEG, PNG, DDS.
     * @param stream A pointer to a stream containing image file
     * @return true on success
     */
    NFE_API_EXPORT UniquePtr<Image> Load(InputStream* stream, ImageFormat destFormat = ImageFormat::Unknown);
    NFE_API_EXPORT bool Load(InputStream* stream, Image* dst, ImageFormat destFormat = ImageFormat::Unknown);

    NFE_API_EXPORT UniquePtr<Image> Empty(const uint32 width, const uint32 height, const ImageFormat format);
    NFE_API_EXPORT bool Empty(const uint32 width, const uint32 height, const ImageFormat format, Image* dst);

    /**
     * Convert RGB pixel values to grayscale.
     * Accepted formats are only: RGB_UByte, RGBA_UByte, RGBA_Float
     * @return true on success
     */
    NFE_API_EXPORT bool Grayscale(Image* src);

} // namespace ImageUtils
} // namespace Common
} // namespace NFE
