/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageConverter class implementation.
 */

#include "../PCH.hpp"
#include "ImageConverter.hpp"
#include "DDSStructures.hpp"

namespace NFE {
namespace Common {

using namespace Math;

ImageConverter::ImageConverter(BufferInputStream* data)
{
    mOriginData = data;
    mOutputData = new (std::nothrow) BufferOutputStream;
}

ImageConverter::~ImageConverter()
{
    Release();
}

void ImageConverter::Release()
{
    mOriginData = nullptr;

    if(!mOutputData)
    {
        delete mOutputData;
        mOutputData = nullptr;
    }
}

BufferInputStream* ImageConverter::GetOriginData() const
{
    return mOriginData;
}

BufferOutputStream* ImageConverter::GetOutputData() const
{
    return mOutputData;
}

int ImageConverter::CompressDDS()
{
    //TBD
    return 0;
}

int ImageConverter::DecompressDDS(ImageFormat &ddsFormat)
{
    switch(ddsFormat)
    {
        case ImageFormat::BC1:
            DecompressDDSBC1();
    };

    return 0;
}

Vector RGB565toVector(uchar* color)
{
    uchar invertedColor = color[1] << 4;
    invertedColor += color[0];
    float red = static_cast<float>((invertedColor >> 7 >> 7) & 0x1F);
    float green = static_cast<float>((invertedColor >> 5) & 0x3F);
    float blue = static_cast<float>(invertedColor & 0x1F);
    float alpha = 255.0f;

    return Vector(red, green, blue, alpha) * g_Byte2Float;
}

int ImageConverter::DecompressDDSBC1()
{
    BC1 ddsStruct;
    uint64 chunksNumber = mOriginData->GetSize() / sizeof(ddsStruct);
    for (int i = 0; i < chunksNumber; i++)
        {
            mOriginData->Read(sizeof(ddsStruct), &ddsStruct);
            Vector color[4];
            color[0] = RGB565toVector(ddsStruct.color1);
            color[1] = RGB565toVector(ddsStruct.color4);
            color[2] = ((2 * color[0]) + color[1]) / 3;
            color[3] = ((2 * color[1]) + color[0]) / 3;

            for (int j = 0; j < 4; j++)
            {
                int colorIndex = 0;
                for (int k = 0; k < 8; k += 2)
                {
                    colorIndex = (ddsStruct.indices[j] >> k) & 0x3;
                    WriteColorToBuffer(color[colorIndex]);
                }
            }
        }

    return 0;
}

void ImageConverter::WriteColorToBuffer(Vector color)
{
    size_t size = sizeof(float);
    for (int i = 0; i < 4; i++)
        mOutputData->Write(&color[i], size);
}



int ImageConverter::Convert(ImageFormat destFormat)
{
    //TBD
    destFormat = ImageFormat::Unknown;
    return 0;
}

} // Common
} // NFE
