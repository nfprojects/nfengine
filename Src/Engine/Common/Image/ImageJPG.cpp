/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageJPG class definition.
 */


#include "PCH.hpp"
#include "Utils/Stream/InputStream.hpp"
#include "Utils/Stream/OutputStream.hpp"
#include "Image.hpp"
#include "ImageJPG.hpp"
#include "Reflection/ReflectionClassDefine.hpp"

#include "jpeg/jpgd.h"
#include "jpeg/jpge.h"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Common::ImageJPG)
    NFE_CLASS_PARENT(NFE::Common::ImageType)
NFE_END_DEFINE_CLASS()

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

//////////////////////////////////////////////////////////////////////////

StringView ImageJPG::GetName() const
{
    const StringView name("JPG");
    return name;
}

bool ImageJPG::Check(InputStream* stream)
{
    uint32 signature = 0;
    stream->Seek(0, SeekMode::Begin);
    if (sizeof(signature) < stream->Read(&signature, sizeof(signature)))
    {
        NFE_LOG_ERROR("Could not read signature from the stream.");
        return false;
    }

    stream->Seek(0, SeekMode::Begin);

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
    NFE_UNUSED(img);
    NFE_UNUSED(stream);

    return false;
}

} // namespace Common
} // namespace NFE
