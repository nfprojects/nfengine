/**
 * @file
 * @author mkkulagowski
 * @brief  Mipmap class declaration.
 */

#pragma once

//#include "Image.hpp"
#include "ImageFormat.hpp"
#include "../InputStream.hpp"
#include "../OutputStream.hpp"
#include "../Math/Math.hpp"


namespace NFE {
namespace Common {

using namespace Math;
//class BufferInputStream;
//class BufferOutputStream;

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
    //const size_t GetOriginDataSize();
    //const size_t GetOutputDataSize();

    int SetOriginData(BufferInputStream* data);
    //int SetOriginDataSize(size_t dataSize);

    int CompressDDS();
    int DecompressDDS(ImageFormat &ddsFormat);
    int Convert(ImageFormat destFormat);
};

} // namespace Common
} // namespace NFE