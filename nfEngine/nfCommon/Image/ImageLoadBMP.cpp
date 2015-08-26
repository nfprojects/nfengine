/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LoadBMP function definition from Image class.
 */

#include "../PCH.hpp"
#include "Image.hpp"
#include "../Logger.hpp"
#include "../Bit.hpp"

namespace NFE {
namespace Common {

#pragma pack(push, 2)
struct BitmapFileHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offBits;
};
#pragma pack(pop)

struct CIEXYZ {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct CIEXYZTriple {
    CIEXYZ red;
    CIEXYZ green;
    CIEXYZ blue;
};

struct BitmapV5Header {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t sizeImage;
    uint32_t xPelsPerMeter;
    uint32_t yPelsPerMeter;
    uint32_t clrUsed;
    uint32_t clrImportant;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
    uint32_t cSType;
    CIEXYZTriple ciexyzEndpoints;
    uint32_t gammaRed;
    uint32_t gammaGreen;
    uint32_t gammaBlue;
    uint32_t intent;
    uint32_t profileData;
    uint32_t profileSize;
    uint32_t reserved;
};

struct RGBQuad {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
};

bool GetColorPalette(InputStream* stream, std::vector<RGBQuad>& palette);
bool ReadPixelsWithPalette(InputStream* stream, size_t offset, uint32 width,
                            uint32 height, uint8_t bitsPerPixel, std::vector<Mipmap> &dest,
                            std::vector<RGBQuad> &palette);
bool ReadPixels(InputStream* stream, size_t offset, uint32 width, uint32 height,
                            uint8_t bitsPerPixel, std::vector<Mipmap> &dest, uint32_t* colorMask);

bool Image::LoadBMP(InputStream* stream)
{
    BitmapFileHeader fileHeader;
    BitmapV5Header infoHeader;
    std::vector<RGBQuad> palette;

    // Check for buffer too small
    if (stream->GetSize() < sizeof(BitmapFileHeader) + sizeof(BitmapV5Header))
    {
        LOG_ERROR("Stream is not big enough to hold even sole BMP headers.");
        return false;
    }

    // Read file header
    stream->Read(sizeof(BitmapFileHeader), &fileHeader);

    // Check for signature mismatch
    if (fileHeader.type != 0x4D42)
    {
        LOG_ERROR("BMP signature mismatch.");
        return false;
    }
    // Check for wrong file size
    if (fileHeader.size > stream->GetSize())
    {
        LOG_ERROR("BMP header filesize does not match the size of the stream.");
        return false;
    }

    // Read info header
    stream->Read(sizeof(BitmapV5Header), &infoHeader);

    uint8_t bitsPerPixel = static_cast<uint8_t>(infoHeader.bitCount);
    bool paletteUsed = bitsPerPixel <= 8;

    // Check if BMP contains palette
    if (paletteUsed)
    {
        // Jump to the palette
        uint32 paletteOffset = sizeof(BitmapFileHeader) + infoHeader.size;
        stream->Seek(paletteOffset);

        // Resize & fill the palette
        palette.resize(infoHeader.clrUsed);
        if (!GetColorPalette(stream, palette))
        {
            LOG_ERROR("Palette could not be read");
            return false;
        }
    }

    // Create color masks
    uint32_t colorMask[4];
    colorMask[0] = infoHeader.redMask;
    colorMask[1] = infoHeader.greenMask;
    colorMask[2] = infoHeader.blueMask;
    colorMask[3] = infoHeader.alphaMask;

    // Jump to the pixels
    stream->Seek(fileHeader.offBits);
    size_t offset = fileHeader.offBits;

    mWidth = infoHeader.width;
    mHeight = infoHeader.height;

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

// Function to load color palette
bool GetColorPalette(InputStream* stream, std::vector<RGBQuad>& palette)
{
    size_t sizeToRead = sizeof(RGBQuad) * palette.size();
    stream->Read(sizeToRead, palette.data());

    return true;
}

// Function to read pixels for BMPs with >8bpp
bool ReadPixels(InputStream* stream, size_t offset, uint32 width, uint32 height,
                uint8_t bitsPerPixel, std::vector<Mipmap> &dest, uint32_t* colorMask)
{
    uint8_t colorSize = bitsPerPixel / 8;
    size_t dataSize = width * height * 4;

    int lineSize = width * colorSize;
    int lineSizeActual = lineSize;
    while (lineSizeActual % 4)
        lineSizeActual++;

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
        std::unique_ptr<uint8_t[]> colorData(new (std::nothrow) uint8_t[lineSize]);

        for (int y = static_cast<int>(height - 1); y >= 0; y--)
        {
            // Read single line
            if (stream->Read(lineSize, colorData.get()) != lineSize)
            {
                LOG_ERROR("Pixels read wrong.");
                return false;
            }

            for (int x = 0; x < static_cast<int>(width); x++)
            {
                // Store 3 colors - RGB
                for (uint32 i = 0; i < 3; i++)
                    imageData.get()[4 * (y * width + x) + i] = colorData[x * colorSize + (2 - i)];

                // Store 255 for alpha channel
                imageData.get()[4 * (y * width + x) + 3] = 255;
            }
            offset += lineSizeActual;
            stream->Seek(offset);
        }
    }
    else
    {
        std::unique_ptr<uint32_t[]> colorData(new (std::nothrow) uint32_t[lineSize]);
        uint8_t colorsPer4Bytes = 4 / colorSize;
        uint32_t colorsMask = CreateBitMask(bitsPerPixel);
        int index = 0;
        for (int y = static_cast<int>(height - 1); y >= 0; y--)
        {
            // Read single line
            stream->Read(lineSize, colorData.get());
            // Create variable for storing single pixel data in 32bits format
            uint32_t colorData32 = 0;
            for (int x = 0; x < static_cast<int>(width); x += colorsPer4Bytes)
            {
                // Read 2 or 4 bytes, depending on 16 or 32 bpp
                for (int j = 0; j < static_cast<int>(colorsPer4Bytes); j++)
                {
                    index = x / colorsPer4Bytes;
                    colorData32 = ((colorData[index] >> (j * bitsPerPixel)) & colorsMask);

                    // Get RGBA values from read bytes
                    for (uchar i = 0; i < 4; i++)
                    {
                        // Count trailing zeros for current colorMask
                        uint8_t maskOffset = CountTrailingZeros(colorMask[i]);
                        // Get color value (it may be 5, 6 or 8 bits)
                        uint32_t singleColor = (colorData32 & colorMask[i]) >> maskOffset;

                        // If color mask is not zero, then normalize value to 0-255 range
                        if (colorMask[i])
                        {
                            singleColor *= 255;
                            singleColor /= (colorMask[i] >> maskOffset);
                        }
                        // Store color data
                        uint8_t singleColor8b = static_cast<uint8_t>(singleColor);
                        imageData.get()[4 * (y * width + x + j) + i] = singleColor8b;
                    }

                    // If there was no alpha mask (no alpha stored in file), store 255
                    if (!colorMask[3])
                        imageData.get()[4 * (y * width + x + j) + 3] = 255;
                }
            }
            offset += lineSizeActual;
            stream->Seek(offset);
        }
    }

    dest.push_back(std::move(Mipmap(imageData.get(), width, height, dataSize)));
    return true;
}

// Function to read pixels for BMPs with <=8bpp (these contain color palette)
bool ReadPixelsWithPalette(InputStream* stream, size_t offset, uint32 width,
                            uint32 height, uint8_t bitsPerPixel, std::vector<Mipmap> &dest,
                            std::vector<RGBQuad> &palette)
{
    size_t dataSize = width * height * 4;
    uint8_t colorsPerByte = 8 / bitsPerPixel;
    uint8_t bitMask = static_cast<uint8_t>(CreateBitMask(bitsPerPixel));

    int lineSize = width / colorsPerByte;
    int lineSizeActual = lineSize;
    while (lineSizeActual % 4)
        lineSizeActual++;

    std::unique_ptr<uchar[]> imageData(new (std::nothrow) uchar[dataSize]);
    std::unique_ptr<uint8_t[]> colorData(new (std::nothrow) uint8_t[lineSize]);

    if (!imageData.get())
    {
        LOG_ERROR("Allocating memory for loading BMP image failed.");
        return false;
    }

    // Number of bytes used to hold one line
    int bytesNumber = (width + (width % colorsPerByte)) / colorsPerByte;

    for (int y = static_cast<int>(height - 1); y >= 0; y--)
    {
        // Read one byte of data
        if (stream->Read(lineSize, colorData.get()) != lineSize)
        {
            LOG_ERROR("Pixels read wrong.");
            return false;
        }
        
        // Pixel's position in line
        uint32_t x = 0;
        for (int i = 0; i < bytesNumber; i++)
        {
            for (uint8_t j = 0; j < colorsPerByte && x < width; j++)
            {
                int paletteIndex = (colorData[i] >> (bitsPerPixel * j)) & bitMask;
                int imageDataIndex = 4 * (y * width + x);
                imageData.get()[imageDataIndex + 0] = palette[paletteIndex].rgbRed;
                imageData.get()[imageDataIndex + 1] = palette[paletteIndex].rgbGreen;
                imageData.get()[imageDataIndex + 2] = palette[paletteIndex].rgbBlue;
                imageData.get()[imageDataIndex + 3] = 255;
                x++;
            }
        }
        offset += lineSizeActual;
        stream->Seek(offset);
    }

    dest.push_back(std::move(Mipmap(imageData.get(), width, height, dataSize)));
    return true;
}
} // namespace Common
} // namespace NFE