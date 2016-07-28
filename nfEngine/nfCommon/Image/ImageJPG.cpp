/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Load/SaveJPG function definition from Image class.
 */


#include "../PCH.hpp"
#include "Image.hpp"
#include "../Logger.hpp"
#include "jpeg/jpgd.h"
#include "jpeg/jpge.h"

namespace NFE {
namespace Common {

// Custom class to allow reading with our InputStream class
class CustomJpegIStream : public jpgd::jpeg_decoder_stream
{
private:
    InputStream* mStream;

public:
    CustomJpegIStream(InputStream* stream)
    {
        mStream = stream;
    }

    int read(jpgd::uint8* pBuf, int max_bytes_to_read, bool* pEOF_flag)
    {
        size_t bytesRead = mStream->Read(max_bytes_to_read, pBuf);
        if (bytesRead == 0)
            *pEOF_flag = true;

        return static_cast<int>(bytesRead);
    }
};

// Custom class to allow writing with our OutputStream class
class CustomJpegOStream : public jpge::output_stream
{
private:
    OutputStream* mStream;

public:
    CustomJpegOStream(OutputStream* stream)
    {
        mStream = stream;
    }

    bool put_buf(const void* Pbuf, int len) override
    {
        return len == mStream->Write(Pbuf, len);
    }
};

bool Image::LoadJPG(InputStream* stream)
{
    CustomJpegIStream jpegStream(stream);

    int comps;
    void* data = jpgd::decompress_jpeg_image_from_stream(&jpegStream, &mWidth, &mHeight, &comps, 3);
    size_t dataSize = mWidth * mHeight * 3;
    Mipmap mipmap(data, mWidth, mHeight, dataSize);
    free(data);
    mFormat = ImageFormat::RGB_UByte;
    mMipmaps.push_back(mipmap);
    return true;
}

bool Image::SaveJPG(OutputStream* stream)
{
    CustomJpegOStream jpegStream(stream);
    jpge::jpeg_encoder jpegEncoder;
    jpge::params jpegParams;
    jpegParams.m_quality = 100;

    size_t bitsPerPix = BitsPerPixel(mFormat);

    if (!jpegEncoder.init(&jpegStream, mWidth, mHeight, static_cast<int>(bitsPerPix / 8), jpegParams))
    {
        LOG_ERROR("Error while initializing encoder");
        jpegEncoder.deinit();
        return false;
    }
    size_t lineSize = bitsPerPix / 8 * mWidth * sizeof(uint8);
    size_t streamSize = mMipmaps[0].mData.GetSize() / sizeof(uint8);
    uint8* linePtr = reinterpret_cast<uint8*>(mMipmaps[0].mData.GetData());
    for (size_t i = 0; i < streamSize; i += lineSize)
        if (!jpegEncoder.process_scanline(linePtr + i))
        {
            LOG_ERROR("Error while processing line no. %i", i / lineSize);
            jpegEncoder.deinit();
            return false;
        }

    if (!jpegEncoder.process_scanline(nullptr))
    {
        LOG_ERROR("Error while finishing compression");
        jpegEncoder.deinit();
        return false;

    }

    jpegEncoder.deinit();
    return true;
}

} // namespace Common
} // namespace NFE
