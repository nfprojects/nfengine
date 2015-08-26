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
} BITMAPFILEHEADER;

typedef struct {
    uint32_t bV5Size;
    uint32_t bV5Width;
    uint32_t bV5Height;
    uint16_t bV5Planes;
    uint16_t bV5BitCount;
    uint32_t bV5Compression;
    uint32_t bV5SizeImage;
    uint32_t bV5XPelsPerMeter;
    uint32_t bV5YPelsPerMeter;
    uint32_t bV5ClrUsed;
    uint32_t bV5ClrImportant;
    uint32_t bV5RedMask;
    uint32_t bV5GreenMask;
    uint32_t bV5BlueMask;
    uint32_t bV5AlphaMask;
    uint32_t bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    uint32_t bV5GammaRed;
    uint32_t bV5GammaGreen;
    uint32_t bV5GammaBlue;
    uint32_t bV5Intent;
    uint32_t bV5ProfileData;
    uint32_t bV5ProfileSize;
    uint32_t bV5Reserved;
} BITMAPV5HEADER;

typedef struct {
    CIEXYZ ciexyzRed;
    CIEXYZ ciexyzGreen;
    CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;

typedef struct {
    FXPT2DOT30 ciexyzX;
    FXPT2DOT30 ciexyzY;
    FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef long FXPT2DOT30;

typedef struct {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;

uchar CreateBitMask(int length);
bool GetColorPalette(InputStream* stream, std::vector<RGBQUAD>& palette);
bool ReadPixelsWithPalette(InputStream* stream, size_t offset, uint32 width, uint32 height,
                    uint32 bitsPerPixel, std::vector<Mipmap> &dest, std::vector<RGBQUAD> &palette);
bool ReadPixels(InputStream* stream, size_t offset, uint32 width, uint32 height,
                    uint32 bitsPerPixel, std::vector<Mipmap> &dest, uint32_t* colorMask);
uint8_t CountTrailingZeros(uint32_t &bytes);

bool Image::LoadBMP(InputStream* stream)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPV5HEADER infoHeader;
    std::vector<RGBQUAD> palette;

    // Check for buffer too small
    if (stream->GetSize() < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV5HEADER))
        return false;

    // Read each member of file header manually, because reading whole in one go
    // losts 2 bytes of data, thanks to the padding
    stream->Read(sizeof(uint16_t), &fileHeader.bfType);
    stream->Read(sizeof(uint32_t), &fileHeader.bfSize);
    stream->Read(sizeof(uint16_t), &fileHeader.bfReserved1);
    stream->Read(sizeof(uint16_t), &fileHeader.bfReserved2);
    stream->Read(sizeof(uint32_t), &fileHeader.bfOffBits);

    // Check for signature mismatch
    if (fileHeader.bfType != 0x4D42)
        return false;
    // Check for wrong file size
    if (fileHeader.bfSize > stream->GetSize())
        return false;

    // Read info header
    stream->Read(sizeof(BITMAPV5HEADER), &infoHeader);

    int bitsPerPixel = infoHeader.bV5BitCount;
    bool paletteUsed = bitsPerPixel <= 8;

    // Check if BMP contains palette
    if (paletteUsed)
    {
        // Jump to the palette
        uint32 paletteOffset = 14 + infoHeader.bV5Size;
        stream->Seek(paletteOffset);

        // Resize & fill the palette
        palette.resize(infoHeader.bV5ClrUsed);
        if (!GetColorPalette(stream, palette))
        {
            LOG_ERROR("Palette could not be read");
            return false;
        }
    }

    // Create color masks
    uint32_t colorMask[4];
    colorMask[0] = infoHeader.bV5RedMask;
    colorMask[1] = infoHeader.bV5GreenMask;
    colorMask[2] = infoHeader.bV5BlueMask;
    colorMask[3] = infoHeader.bV5AlphaMask;

    // Jump to the pixels
    stream->Seek(fileHeader.bfOffBits);
    size_t offset = fileHeader.bfOffBits;

    mWidth = infoHeader.bV5Width;
    mHeight = infoHeader.bV5Height;

    // Read pixels
    if (paletteUsed)
    {
        if (!ReadPixelsWithPalette(stream, offset, mWidth, mHeight,
                                    bitsPerPixel, mMipmaps, palette))
        {
            LOG_ERROR("Error while reading pixels");
            Release();
            return false;
        }
    }
    else
    {
        if (!ReadPixels(stream, offset, mWidth, mHeight,
                        bitsPerPixel, mMipmaps, colorMask))
        {
            LOG_ERROR("Error while reading pixels");
            Release();
            return false;
        }
    }

    mFormat = ImageFormat::RGBA_UByte;
    return true;
}

// Function to create continuous bitmask of given length
uchar CreateBitMask(int length)
{
    uchar mask = 0;
    for (int i = 0; i < length; i++)
        mask = (mask << 1) | 0x1;
    return mask;
}

// Function to load color palette
bool GetColorPalette(InputStream* stream, std::vector<RGBQUAD>& palette)
{
    for (auto& i : palette)
    {
        RGBQUAD color;
        stream->Read(sizeof(RGBQUAD), &color);
        i = color;
    }

    return true;
}

// Function to read pixels for BMPs with >8bpp
bool ReadPixels(InputStream* stream, size_t offset, uint32 width, uint32 height,
                uint32 bitsPerPixel, std::vector<Mipmap> &dest, uint32_t* colorMask)
{
    uint32 colorSize = bitsPerPixel / 8;
    size_t dataSize = width * height * 4;

    int lineSize = width * colorSize;
    while (lineSize % 4)
        lineSize++;

    std::unique_ptr<uchar[]> imageData(new (std::nothrow) uchar[dataSize]);

    if (!imageData.get())
    {
        LOG_ERROR("Allocating memory for loading BMP image failed.");
        return false;
    }

    // 24bpp stores no color masks, maybe due to having only 1 possible bit alignment.
    // Either way, that enforces me to make it a separate case
    if (bitsPerPixel == 24)
    {
        std::unique_ptr<uint8_t[]> colorData;

        for (int y = static_cast<int>(height - 1); y >= 0; y--)
        {
            for (int x = 0; x < static_cast<int>(width); x++)
            {
                // Read 3 bytes
                colorData.reset(new (std::nothrow) uint8_t[colorSize]);
                stream->Read(colorSize, colorData.get());

                // Store 3 colors - RGB
                for (uint32 i = 0; i < 3; i++)
                    imageData.get()[4 * (y * width + x) + i] = colorData[2 - i];

                // Store 255 for alpha channel
                imageData.get()[4 * (y * width + x) + 3] = 255;
            }
            offset += lineSize;
            stream->Seek(offset);
        }
    }
    else
    {
        for (int y = static_cast<int>(height - 1); y >= 0; y--)
        {
            for (int x = 0; x < static_cast<int>(width); x++)
            {
                // Read 2 or 4 bytes, depending on 16 or 32 bpp
                uint32_t colorData32 = 0;
                stream->Read(colorSize, &colorData32);

                // Getting RGBA values from read bytes
                for (uchar i = 0; i < 4; i++)
                {
                    // Counting trailing zeros for current colorMask
                    uint8_t maskOffset = CountTrailingZeros(colorMask[i]);
                    // Getting color value (it may be 5, 6 or 8 bits)
                    uint32_t singleColor = (colorData32 & colorMask[i]) >> maskOffset;

                    // If color mask is not zero, then normalize value to 0-255 range
                    if (colorMask[i])
                    {
                        singleColor *= 255;
                        singleColor /= (colorMask[i] >> maskOffset);
                    }
                    // Store color data
                    imageData.get()[4 * (y * width + x) + i] = static_cast<uint8_t>(singleColor);
                }

                // If there was no alpha mask (no alpha stored in file), store 255
                if (!colorMask[3])
                    imageData.get()[4 * (y * width + x) + 3] = 255;
            }
            offset += lineSize;
            stream->Seek(offset);
        }
    }

    dest.push_back(Mipmap(imageData.get(), width, height, dataSize));
    return true;
}

// Function to read pixels for BMPs with <=8bpp (these contain color palette)
bool ReadPixelsWithPalette(InputStream* stream, size_t offset, uint32 width, uint32 height,
                      uint32 bitsPerPixel, std::vector<Mipmap> &dest, std::vector<RGBQUAD> &palette)
{
    size_t dataSize = width * height * 4;
    uint32 colorsPerByte = 8 / bitsPerPixel;
    uchar bitMask = CreateBitMask(bitsPerPixel);

    int lineSize = width / colorsPerByte;
    while (lineSize % 4)
        lineSize++;

    std::unique_ptr<uchar[]> imageData(new (std::nothrow) uchar[dataSize]);
    if (!imageData.get())
    {
        LOG_ERROR("Allocating memory for loading BMP image failed.");
        return false;
    }

    for (int y = static_cast<int>(height - 1); y >= 0; y--)
    {
        for (int x = 0; x < static_cast<int>(width); x += colorsPerByte)
        {
            // Read one byte of data
            uchar colorData;
            stream->Read(sizeof(uchar), &colorData);

            // Read all the pixels stored in loaded byte (there can be 1, 2, 4 or even 8)
            for (uint32 i = 0; i < colorsPerByte; i++)
            {
                int index = (colorData >> (bitsPerPixel * i)) & bitMask;
                imageData.get()[4 * (y * width + x + i) + 0] = palette[index].rgbRed;
                imageData.get()[4 * (y * width + x + i) + 1] = palette[index].rgbGreen;
                imageData.get()[4 * (y * width + x + i) + 2] = palette[index].rgbBlue;
                imageData.get()[4 * (y * width + x + i) + 3] = 255;
            }
        }
        offset += lineSize;
        stream->Seek(offset);
    }

    dest.push_back(Mipmap(imageData.get(), width, height, dataSize));
    return true;
}

// Function used to count trailing zeros
uint8_t CountTrailingZeros(uint32_t &bytes)
{
    if (bytes == 0)
        return 0;

    uint32_t number = bytes;
    uint8_t counter = 0;

    while (!(number & 0x1))
    {
        counter++;
        number >>= 1;
    }

    return counter;
}

} // namespace Common
} // namespace NFE