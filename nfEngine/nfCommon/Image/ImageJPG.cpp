/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageJPG class definition.
 */


#include "../PCH.hpp"
#include "../Logger.hpp"
#include "Image.hpp"
#include "ImageJPG.hpp"
#include "jpeg/jpgd.h"
#include "jpeg/jpge.h"


namespace NFE {
namespace Common {


namespace {
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
        size_t bytesRead = mStream->Read(pBuf, max_bytes_to_read);
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
        size_t toWrite = static_cast<size_t>(len);
        return toWrite == mStream->Write(Pbuf, toWrite);
    }
};

} // namespace

// Register JPG image type
bool gImageJPGRegistered = ImageType::RegisterImageType("JPG", std::make_unique<ImageJPG>());

bool ImageJPG::Check(InputStream* stream)
{
    uint32 signature = 0;
    stream->Seek(0);
    if (sizeof(signature) < stream->Read(&signature, sizeof(signature)))
    {
        LOG_ERROR("Could not read signature from the stream.");
        return false;
    }

    stream->Seek(0);

    return (signature & 0xFFFFFF) == 0xFFD8FF;
}

bool ImageJPG::Load(Image* img, InputStream* stream)
{
    CustomJpegIStream jpegStream(stream);

    int comps, width, height;
    void* data = jpgd::decompress_jpeg_image_from_stream(&jpegStream, &width, &height, &comps, 3);

    bool result = img->SetData(data, width, height, ImageFormat::RGB_UByte);
    free(data);

    return result;
}

bool ImageJPG::Save(Image* img, OutputStream* stream)
{
    if (img->GetFormat() != ImageFormat::RGBA_UByte)
    {
        LOG_ERROR("To save image as JPG, RGBA_UByte format is needed.");
        return false;
    }

    CustomJpegOStream jpegStream(stream);
    jpge::jpeg_encoder jpegEncoder;
    jpge::params jpegParams;
    jpegParams.m_quality = 100;

    uint32 width = img->GetWidth();
    uint32 height = img->GetHeight();

    size_t bitsPerPix = BitsPerPixel(img->GetFormat());

    if (!jpegEncoder.init(&jpegStream, width, height, static_cast<int>(bitsPerPix / 8), jpegParams))
    {
        LOG_ERROR("Error while initializing encoder");
        jpegEncoder.deinit();
        return false;
    }

    size_t lineSize = bitsPerPix / 8 * width * sizeof(uint8);
    size_t streamSize = img->GetMipmap()->GetDataSize() / sizeof(uint8);
    const uint8* linePtr = reinterpret_cast<const uint8*>(img->GetMipmap()->GetData());
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
