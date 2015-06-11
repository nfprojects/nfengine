/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageConverter class definitions.
 */

#pragma once

#include "ImageFormat.hpp"
#include "../InputStream.hpp"
#include "../OutputStream.hpp"
#include "../Math/Math.hpp"


namespace NFE {
namespace Common {

using namespace Math;
typedef Vector Color;

class ImageConverter
{
private:
    std::vector<uchar> mOriginData;
    std::vector<uchar> mOutputData;

    int DecompressDDSBC1();
    void WriteColorToBuffer(Color &color);
    void Release();
public:
    ImageConverter(void* data, size_t dataSize);
    ImageConverter(const ImageConverter& src);
    ~ImageConverter();

    const void* GetOriginData() const;
    const void* GetOutputData() const;

    /**
     * Set data to operate on.
     * @param data Pointer to data.
     * @return 0 on success
     */
    int SetOriginData(void* data);

    /**
     * Compress RGBA stream to DDS format.
     * @return 0 on success
     */
    int CompressDDS();

    /**
     * Decompress DDS stream to RGBA format.
     * @param ddsFormat Format of DDS file compression
     * @return 0 on success
     */
    int DecompressDDS(ImageFormat &ddsFormat);

    /**
     * Convert image to destination format.
     * @param destFormat Destination format.
     * @return 0 on success
     */
    int Convert(ImageFormat destFormat);
};

} // namespace Common
} // namespace NFE