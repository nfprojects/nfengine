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

int Image::LoadBMP(InputStream* pStream)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    // buffer too small
    if (pStream->GetSize() < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
        return 1;

    // read file header
    pStream->Read(sizeof(BITMAPFILEHEADER), &fileHeader);

    if (fileHeader.bfType != 0x4D42) //signature mismatch
        return 1;
    if (fileHeader.bfSize > pStream->GetSize()) //wrong file size
        return 1;

    // read info header
    pStream->Read(sizeof(BITMAPINFOHEADER), &infoHeader);

    // Size of one line in bytes. For BMP it must be multiple of 4
    int lineSize;


    pStream->Seek(fileHeader.bfOffBits);
    size_t offset = fileHeader.bfOffBits;

    mWidth = infoHeader.biWidth;
    mHeight = infoHeader.biWidth;
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
            return 1;
        }

        lineSize = mWidth * 3;
        while (lineSize % 4)
            lineSize++;

        for (int y = 0; y < mHeight; y++)
        {
            for (int x = 0; x < mWidth; x++)
            {
                uchar tmp[3];
                pStream->Read(3, tmp);

                pImageData.get()[4 * (y * mWidth + x)    ] = tmp[2];
                pImageData.get()[4 * (y * mWidth + x) + 1] = tmp[1];
                pImageData.get()[4 * (y * mWidth + x) + 2] = tmp[0];
                pImageData.get()[4 * (y * mWidth + x) + 3] = 255;
            }
            offset += lineSize;
            pStream->Seek(offset);
        }

        mMipmaps.push_back(Mipmap(pImageData.get(), mWidth, mHeight, dataSize));
    }
    else
    {
        Release();
        return 1;
    }

    return 0;
}

} // namespace Common
} // namespace NFE