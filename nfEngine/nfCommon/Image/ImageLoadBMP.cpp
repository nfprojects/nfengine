/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LoadBMP function definition from Image class.
 */

#include "../PCH.hpp"
#include "Image.hpp"
#include "../Logger.hpp"

namespace NFE {
namespace Common {

typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFHEADER;

typedef struct {
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPIHEADER;


bool Image::LoadBMP(InputStream* stream)
{
	BITMAPFHEADER fileHeader;
	BITMAPIHEADER infoHeader;

    // buffer too small
    if (stream->GetSize() < sizeof(BITMAPFHEADER) + sizeof(BITMAPIHEADER))
        return false;

    // read each member of file header manually, because reading whole in one go
	// losts 2 bytes of data, thanks to the padding
    stream->Read(sizeof(uint16_t), &fileHeader.bfType);
	stream->Read(sizeof(uint32_t), &fileHeader.bfSize);
	stream->Read(sizeof(uint16_t), &fileHeader.bfReserved1);
	stream->Read(sizeof(uint16_t), &fileHeader.bfReserved2);
	stream->Read(sizeof(uint32_t), &fileHeader.bfOffBits);

    if (fileHeader.bfType != 0x4D42) //signature mismatch
        return false;
    if (fileHeader.bfSize > stream->GetSize()) //wrong file size
        return false;

    // read info header
    stream->Read(sizeof(BITMAPIHEADER), &infoHeader);

    // Size of one line in bytes. For BMP it must be multiple of 4
    int lineSize;


    stream->Seek(fileHeader.bfOffBits);
    size_t offset = fileHeader.bfOffBits;

    mWidth = infoHeader.biWidth;
    mHeight = infoHeader.biWidth;
	
	// Types of BMPs bitcounts, for future use
    // 24bit - 8B:8G:8R
    // 32bit - 8B:8G:8R:8A
    // 16bit - 5B:5G:5R:1A
    // 8bit - 256*8B:8G:8R pallete + each color 8bit index
    // 4bit - 16*8B:8G:8R pallete + each color 4bit index
    if (infoHeader.biBitCount == 24)
    {
        mFormat = ImageFormat::RGBA_UByte;
        size_t dataSize = mWidth * mHeight * 4;
        std::unique_ptr<uchar[]> pImageData(new (std::nothrow) uchar[dataSize]);

        if (!pImageData.get())
        {
            LOG_ERROR("Allocating memory for loading BMP image failed.");
            Release();
            return false;
        }

        lineSize = mWidth * 3;
        while (lineSize % 4)
            lineSize++;

        for (int y = mHeight - 1; y >= 0; y--)
        {
            for (int x = 0; x < mWidth; x++)
            {
                uchar tmp[3];
                stream->Read(3, tmp);

                pImageData.get()[4 * (y * mWidth + x)    ] = tmp[2];
                pImageData.get()[4 * (y * mWidth + x) + 1] = tmp[1];
                pImageData.get()[4 * (y * mWidth + x) + 2] = tmp[0];
                pImageData.get()[4 * (y * mWidth + x) + 3] = 255;
            }
            offset += lineSize;
            stream->Seek(offset);
        }

        mMipmaps.push_back(Mipmap(pImageData.get(), mWidth, mHeight, dataSize));
    }
    else
    {
        LOG_ERROR("BMP files other than 24bit are not supported.");
        Release();
        return false;
    }

    return true;
}

} // namespace Common
} // namespace NFE