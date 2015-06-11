/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LoadBMP function definition from Image class.
 */

#include "../PCH.hpp"
#include "Image.hpp"

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

    if (infoHeader.biBitCount == 24)
    {
        mFormat = ImageFormat::RGBA_UByte;
        size_t dataSize = mWidth * mHeight * 4;
        std::unique_ptr<uchar> pImageData;
        pImageData.reset(new (std::nothrow) uchar[dataSize]);

        if (!pImageData.get())
        {
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

                // TODO check - swapped tmp[0] and tmp[2], cause of color inversion
                pImageData.get()[4 * (y * mWidth + x)    ] = tmp[2]; // ((uchar*)pData)[offset + 3*x + 2];
                pImageData.get()[4 * (y * mWidth + x) + 1] = tmp[1]; // ((uchar*)pData)[offset + 3*x + 1];
                pImageData.get()[4 * (y * mWidth + x) + 2] = tmp[0]; // ((uchar*)pData)[offset + 3*x    ];
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