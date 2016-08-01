/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LoadJPG function definition from Image class.
 */


#include "../PCH.hpp"
#include "Image.hpp"
#include "jpeg/jpgd.h"

namespace NFE {
namespace Common {

// bridge between InputStream and jpgd::decompress_jpeg_image_from_stream function
class CustomJpegStream : public jpgd::jpeg_decoder_stream
{
private:
    InputStream* mStream;

public:
    CustomJpegStream(InputStream* stream)
    {
        mStream = stream;
    }

    int read(jpgd::uint8* pBuf, int max_bytes_to_read, bool* pEOF_flag)
    {
        size_t bytesRead = mStream->Read(pBuf, max_bytes_to_read);
        if (bytesRead == 0)
            *pEOF_flag = true;

        return static_cast<int>(bytesRead);
    }
};

bool Image::LoadJPG(InputStream* stream)
{
    CustomJpegStream jpegStream(stream);

    int comps;
    void* data = jpgd::decompress_jpeg_image_from_stream(&jpegStream, &mWidth, &mHeight, &comps,
                  4);
    size_t dataSize = mWidth * mHeight * 4;
    Mipmap mipmap(data, mWidth, mHeight, dataSize);
    free(data);
    mFormat = ImageFormat::RGBA_UByte;
    mMipmaps.push_back(mipmap);
    return true;
}

} // namespace Common
} // namespace NFE
