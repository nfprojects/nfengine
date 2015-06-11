/**
 * @file
 * @author mkkulagowski
 * @brief  Mipmap class declaration.
 */

#pragma once

#include "Image.hpp"
#include "DDSStructures.hpp"

namespace NFE {
namespace Common {

class ImageConverter
{
private:
    BufferInputStream* mOriginData;
    BufferOutputStream* mOutputData;

public:    
    ImageConverter(uchar* data);
    ImageConverter(const ImageConverter& src);
    ~ImageConverter();
	
    const uchar* GetOriginData();
    const uchar* GetOutputData();
    //const size_t GetOriginDataSize();
    //const size_t GetOutputDataSize();
    
    int SetOriginData(uchar* data);
    //int SetOriginDataSize(size_t dataSize);
    
    int CompressDDS();
    int DecompressDDS();
    int Convert(ImageFormat destFormat);
};
