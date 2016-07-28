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


namespace NFE {
namespace Common {

namespace {

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

}

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
    CustomJpegStream jpegStream(stream);

    int comps, width, height;
    void* data = jpgd::decompress_jpeg_image_from_stream(&jpegStream, &width, &height, &comps, 4);

    bool result = img->SetData(data, width, height, ImageFormat::RGBA_UByte);
    free(data);

    return result;
}

bool ImageJPG::Save(Image*, OutputStream*)
{
    return false;
}

} // namespace Common
} // namespace NFE
