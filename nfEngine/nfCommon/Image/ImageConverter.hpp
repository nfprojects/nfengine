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

class ImageConverter
{
private:
    BufferInputStream* mOriginData;
    BufferOutputStream* mOutputData;

    int DecompressDDSBC1();
    void WriteColorToBuffer(Vector color);
    void Release();
public:
    ImageConverter(BufferInputStream* data);
    ImageConverter(const ImageConverter& src);
    ~ImageConverter();

    BufferInputStream* GetOriginData() const;
    BufferOutputStream* GetOutputData() const;

    /**
     * Set data to operate on.
     * @param data Pointer to data.
     * @return 0 on success
     */
    int SetOriginData(BufferInputStream* data);

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