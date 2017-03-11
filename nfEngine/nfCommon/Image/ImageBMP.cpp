/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LoadBMP function definition from Image class.
 */

#include "PCH.hpp"
#include "ImageBMP.hpp"
#include "Image.hpp"
#include "Logger/Logger.hpp"
#include "Utils/Bit.hpp"
#include "Utils/InputStream.hpp"


namespace NFE {
namespace Common {

namespace {

#pragma pack(push, 2)
struct BitmapFileHeader
{
    uint16 type;
    uint32 size;
    uint16 reserved1;
    uint16 reserved2;
    uint32 offBits;
};
#pragma pack(pop)

struct CIEXYZ
{
    uint32 x;
    uint32 y;
    uint32 z;
};

struct CIEXYZTriple
{
    CIEXYZ red;
    CIEXYZ green;
    CIEXYZ blue;
};

struct BitmapV5Header
{
    uint32 size;
    uint32 width;
    uint32 height;
    uint16 planes;
    uint16 bitCount;
    uint32 compression;
    uint32 sizeImage;
    uint32 xPelsPerMeter;
    uint32 yPelsPerMeter;
    uint32 clrUsed;
    uint32 clrImportant;
    uint32 redMask;
    uint32 greenMask;
    uint32 blueMask;
    uint32 alphaMask;
    uint32 cSType;
    CIEXYZTriple ciexyzEndpoints;
    uint32 gammaRed;
    uint32 gammaGreen;
    uint32 gammaBlue;
    uint32 intent;
    uint32 profileData;
    uint32 profileSize;
    uint32 reserved;
};

struct RGBQuad
{
    uint8 rgbBlue;
    uint8 rgbGreen;
    uint8 rgbRed;
    uint8 rgbReserved;
};

// Function to load color palette
bool GetColorPalette(InputStream* stream, std::vector<RGBQuad>& palette)
{
    size_t sizeToRead = sizeof(RGBQuad) * palette.size();
    stream->Read(palette.data(), sizeToRead);

    return true;
}

// Function to read pixels for BMPs with >8bpp
bool ReadPixels(InputStream* stream, size_t offset, uint32 width, uint32 height,
                uint8 bitsPerPixel, Image* img, uint32* colorMask)
{
    uint8 colorSize = bitsPerPixel / 8;
    size_t dataSize = width * height * 4;

    uint32 lineSize = width * colorSize;
    uint32 lineSizeActual = lineSize;
    while (lineSizeActual % 4)
        lineSizeActual++;

    std::unique_ptr<uint8[]> imageData(new (std::nothrow) uint8[dataSize]);

    if (!imageData.get())
    {
        LOG_ERROR("Allocating memory for loading BMP image failed.");
        return false;
    }

    // 24bpp stores no color masks, maybe due to having only 1 possible bit alignment.
    // Either way, that enforces me to make it a separate case
    if (bitsPerPixel == 24)
    {
        std::unique_ptr<uint8[]> colorData(new (std::nothrow) uint8[lineSize]);

        for (int y = static_cast<int>(height - 1); y >= 0; y--)
        {
            // Read single line
            if (stream->Read(colorData.get(), lineSize) != lineSize)
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
        std::unique_ptr<uint32[]> colorData(new (std::nothrow) uint32[lineSize]);
        uint8 colorsPer4Bytes = 4 / colorSize;
        uint32 colorsMask = NFE::Common::CreateBitMask(bitsPerPixel);

        for (int y = static_cast<int>(height - 1); y >= 0; y--)
        {
            // Read single line
            if(stream->Read(colorData.get(), lineSize) != lineSize)
            {
                LOG_ERROR("Pixels read wrong.");
                return false;
            }

            for (uint32 x = 0; x < width; x += colorsPer4Bytes)
            {
                // Read 2 or 4 bytes, depending on 16 or 32 bpp
                for (uint32 j = 0; j < colorsPer4Bytes; j++)
                {
                    uint32 byteIndex = x / colorsPer4Bytes;

                    // Variable for storing single pixel data in 32bits format
                    uint32 colorData32 = ((colorData[byteIndex] >> (j * bitsPerPixel)) & colorsMask);

                    // Get RGBA values from read bytes
                    for (uint8 i = 0; i < 4; i++)
                    {
                        // Count trailing zeros for current colorMask
                        uint8 maskOffset = NFE::Common::CountTrailingZeros(colorMask[i]);

                        // Get color value (it may be 5, 6 or 8 bits)
                        uint32 singleColor = (colorData32 & colorMask[i]) >> maskOffset;

                        // If color mask is not zero, then normalize value to 0-255 range
                        if (colorMask[i])
                        {
                            singleColor *= 255;
                            singleColor /= (colorMask[i] >> maskOffset);
                        }

                        // Store color data
                        uint8 singleColor8b = static_cast<uint8>(singleColor);
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

    img->SetData(imageData.get(), width, height, ImageFormat::RGBA_UByte);
    return true;
}

// Function to read pixels for BMPs with <=8bpp (these contain color palette)
bool ReadPixelsWithPalette(InputStream* stream, size_t offset, uint32 width,
                           uint32 height, uint8 bitsPerPixel, Image* img,
                           std::vector<RGBQuad> &palette)
{
    size_t dataSize = width * height * 4;
    uint8 colorsPerByte = 8 / bitsPerPixel;
    uint8 bitMask = static_cast<uint8>(NFE::Common::CreateBitMask(bitsPerPixel));

    // lineSize is a size of all non-empty bytes in line
    uint32 lineSize = (width + (width % colorsPerByte)) / colorsPerByte;
    uint32 lineSizeActual = lineSize;
    while (lineSizeActual % 4)
        lineSizeActual++;

    std::unique_ptr<uint8[]> imageData(new (std::nothrow) uint8[dataSize]);
    std::unique_ptr<uint8[]> colorData(new (std::nothrow) uint8[lineSize]);

    if (!imageData.get())
    {
        LOG_ERROR("Allocating memory for loading BMP image failed.");
        return false;
    }

    // Number of bytes used to hold one line
    //int bytesNumber = (width + (width % colorsPerByte)) / colorsPerByte;

    for (int y = static_cast<int>(height - 1); y >= 0; y--)
    {
        // Read one byte of data
        if (stream->Read(colorData.get(), lineSize) != lineSize)
        {
            LOG_ERROR("Pixels read wrong.");
            return false;
        }

        for (uint32 x = 0; x < width; x++)
        {
            int byteIndex = x / colorsPerByte;
            int imageDataIndex = 4 * (y * width + x);
            uint8 bitShift = bitsPerPixel * (x % colorsPerByte);

            int paletteIndex = (colorData[byteIndex] >> bitShift) & bitMask;

            imageData.get()[imageDataIndex + 0] = palette[paletteIndex].rgbRed;
            imageData.get()[imageDataIndex + 1] = palette[paletteIndex].rgbGreen;
            imageData.get()[imageDataIndex + 2] = palette[paletteIndex].rgbBlue;
            imageData.get()[imageDataIndex + 3] = 255;
        }
        offset += lineSizeActual;
        stream->Seek(offset);
    }

    return img->SetData(imageData.get(), width, height, ImageFormat::RGBA_UByte);
}

}

// Register BMP image type
bool gImageBMPRegistered = ImageType::RegisterImageType("BMP", std::make_unique<ImageBMP>());

bool ImageBMP::Check(InputStream* stream)
{
    uint16 signature = 0;
    stream->Seek(0);
    if (sizeof(signature) < stream->Read(&signature, sizeof(signature)))
    {
        LOG_ERROR("Could not read signature from the stream.");
        return false;
    }

    stream->Seek(0);

    return signature == 0x4D42;
}

bool ImageBMP::Load(Image* img, InputStream* stream)
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
    stream->Read(&fileHeader, sizeof(BitmapFileHeader));

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
    stream->Read(&infoHeader, sizeof(BitmapV5Header));

    uint8 bitsPerPixel = static_cast<uint8>(infoHeader.bitCount);
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
    uint32 colorMask[4];
    colorMask[0] = infoHeader.redMask;
    colorMask[1] = infoHeader.greenMask;
    colorMask[2] = infoHeader.blueMask;
    colorMask[3] = infoHeader.alphaMask;

    // Jump to the pixels
    stream->Seek(fileHeader.offBits);
    size_t offset = fileHeader.offBits;

    // Read pixels
    bool result;
    if (paletteUsed)
        result = ReadPixelsWithPalette(stream, offset, infoHeader.width, infoHeader.height,
                                       bitsPerPixel, img, palette);
    else
        result = ReadPixels(stream, offset, infoHeader.width, infoHeader.height,
                            bitsPerPixel, img, colorMask);

    if (!result)
    {
        LOG_ERROR("Error while reading pixels");
        img->Release();
    }

    return result;
}

bool ImageBMP::Save(Image*, OutputStream*)
{
    return false;
}

} // namespace Common
} // namespace NFE
