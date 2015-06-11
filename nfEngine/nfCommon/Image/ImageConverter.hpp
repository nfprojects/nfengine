/**
 * @file
 * @author mkkulagowski
 * @brief  Mipmap class declaration.
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

    int SetOriginData(BufferInputStream* data);

    int CompressDDS();
    int DecompressDDS(ImageFormat &ddsFormat);
    int Convert(ImageFormat destFormat);
};

} // namespace Common
} // namespace NFE