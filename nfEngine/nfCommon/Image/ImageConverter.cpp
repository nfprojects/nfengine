#include "../PCH.hpp"
#include "ImageConverter.hpp"

namespace NFE {
namespace Common {

ImageConverter::ImageConverter(uchar* data, size_t dataSize)
{
    mOriginDataSize = dataSize;
    mOriginData = new BufferInputStream(data, dataSize);
    
    mOutputData = nullptr;
    mOutputDataSize = 0;
}

ImageConverter::~ImageConverter()
{
    Release();
}

void ImageConverter::Release()
{
    delete[] mOriginData;
    mOriginData = nullptr;
    mOriginDataSize = 0;
    
    if(!mOutputData)
    {
        delete[] mOutputData;
        mOutputData = nullptr;
        mOutputDataSize = 0;
    }
}

const uchar* ImageConverter::GetOriginData()
{
    return mOriginData;
}

const uchar* ImageConverter::GetOutputData()
{
    return mOutputData;
}

const size_t ImageConverter::GetOriginDataSize()
{
    return mOriginDataSize;
}

const size_t ImageConverter::GetOutputDataSize()
{
    return mOutputDataSize;
}

void ImageConverter::SetOriginData(uchar* data, uint32 dataSize)
{
    Release();
    
    mOriginDataSize = dataSize;
    mOriginData = new uchar*[dataSize];
    memcpy(mOriginData, data, dataSize);
}

int ImageConverter::SetOriginDataSize(size_t dataSize)
{
    mOriginDataSize = dataSize;
}

int ImageConverter::CompressDDS()
{
    
}

int ImageConverter::DecompressDDS(ImageFormat &ddsFormat)
{
    switch(ddsFormat):
    {
        case BC1:
            DecompressDDSBC1();
    };

    return 0;
}

int ImageConverter::DecompressDDSBC1()
{
    BC1 ddsStruct;
    int chunksNumber = mOriginDataSize / sizeof(ddsStruct);
    for (int i = 0; i < chunksNumber; i++)
        {
            mOriginData->Read(sizeof(ddsStruct), *ddsStruct);
            Vector color[4];
            color[0] = RGB565toVector(ddsStruct.color1);
            color[1] = RGB565toVector(ddsStruct.color4);
            color[2] = ((2 * color[0]) + color[1]) / 3f;
            color[3] = ((2 * color[1]) + color[0]) / 3f;
            
            for (int j = 0; j < 4; j++)
                int colorIndex = 0;
                for (int k = 0; k < 8; k+=2)
                    {
                        colorIndex = (indices[j] >> k) & 0x3;
                        WriteColorToBuffer(color[colorIndex]);
                    }         
        }
}

 //size_t Write(const void* pSrc, size_t num)

void WriteColorToBuffer(Vector color)
{
    size_t size = sizeof(float);
    for (int i = 0; i < 4, i++)
        mOutputData->Write(color + (i * size), size);
}

Vector RGB565toVector(uchar* color)
{
    uchar16_t invertedColor = color[1] << 4;
    invertedColor += color[0];
    float red = static_cast<float>((invertedColor >> 11) & 0x1F);
    float green = static_cast<float>((invertedColor >> 5) & 0x3F);
    float blue = static_cast<float>(invertedColor & 0x1F);
    float alpha = 255.0f;
    
    return Vector(red, green, blue, alpha) * g_Byte2Float;
}

int ImageConverter::Convert(ImageFormat &destFormat)
{
    
}
    
} // Common
} // NFE