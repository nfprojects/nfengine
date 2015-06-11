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

ImageConverter::ImageConverter(void* data, size_t dataSize)
{
    mOriginData.reserve(dataSize);
    uchar* dataUchar = static_cast<uchar*>(data);
    mOriginData.assign(dataUchar, dataUchar + dataSize);

    mOutputData.clear();
    mOutputData.shrink_to_fit();
}

ImageConverter::~ImageConverter()
{
    Release();
}

void ImageConverter::Release()
{
    mOriginData.clear();
    mOriginData.shrink_to_fit();

    if(!mOutputData.empty())
    {
        mOutputData.clear();
        mOutputData.shrink_to_fit();
    }
}

const void* ImageConverter::GetOriginData() const
{
    return mOriginData.data();
}

const void* ImageConverter::GetOutputData() const
{
    return mOutputData.data();
}

int ImageConverter::CompressDDS()
{
    //TBD
    return 1;
}

int ImageConverter::DecompressDDS(ImageFormat &ddsFormat)
{
    switch(ddsFormat)
    {
        case ImageFormat::BC1:
            return DecompressDDSBC1();
    };

    return 1;
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
    uint32 structSize = sizeof(ddsStruct);
    uint32 ucharNumber = structSize / sizeof(uchar);
    uint32 chunksNumber = mOriginData.size() / ucharNumber;
    for (uint32 i = 0; i < chunksNumber; i++)
        {
            memcpy(&ddsStruct, mOriginData.data() + (i * structSize), structSize);
            Color color[4];
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

void ImageConverter::WriteColorToBuffer(Color &color)
{
    for (int i = 0; i < 4; i++)
        mOutputData.push_back(static_cast<uchar>(color[i]));
}



int ImageConverter::Convert(ImageFormat destFormat)
{
        //alloc buffer for new format
        size_t newDataSize = mMipmaps[i].width * mMipmaps[i].height * BitsPerPixel(destFormat) / 8;
        void* pNewData = malloc(newDataSize);

        void* pOldData = mMipmaps[i].data;

        uint32 width = mMipmaps[i].height;
        uint32 height = mMipmaps[i].height;
        Vector tmp;

        for (uint32 y = 0; y < height; y++)
        {
            for (uint32 x = 0; x < width; x++)
            {
                tmp = GetTexel(pOldData, x, y, width, mFormat);
                SetTexel(tmp, pNewData, x, y, width, destFormat);
            }
        }

        free(mMipmaps[i].data);
        mMipmaps[i].data = pNewData;
        mMipmaps[i].dataSize = newDataSize;

    return 0;
}

} // Common
} // NFE
