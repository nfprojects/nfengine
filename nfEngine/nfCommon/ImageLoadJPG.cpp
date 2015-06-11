/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Image function definitions.
 */

#include "PCH.hpp"
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
    CustomJpegStream(InputStream* pStream)
    {
        mStream = pStream;
    }

    int read(jpgd::uint8* pBuf, int max_bytes_to_read, bool* pEOF_flag)
    {
        size_t bytesRead = mStream->Read(max_bytes_to_read, pBuf);
        if (bytesRead == 0)
            *pEOF_flag = true;

        return static_cast<int>(bytesRead);
    }
};

int Image::LoadJPEG(InputStream* pStream)
{
    CustomJpegStream jpegStream(pStream);

    int comps;
    ImageMipmap mipmap;
    mipmap.data = jpgd::decompress_jpeg_image_from_stream(&jpegStream, &mWidth, &mHeight, &comps,
                  4);
    mipmap.dataSize = mWidth * mHeight * 4;
    mipmap.width = mWidth;
    mipmap.height = mHeight;

    if (mipmap.data)
    {
        mFormat = ImageFormat::RGBA_UByte;
        mMipmaps.push_back(mipmap);
        return 0;
    }
    else
        return 1;
}

} // namespace Common
} // namespace NFE