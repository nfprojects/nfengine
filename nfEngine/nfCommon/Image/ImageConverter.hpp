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

enum class ImageFormat;
class BufferInputStream;
class BufferOutputStream;

class ImageConverter
{
private:
    BufferInputStream* mOriginData;
    BufferOutputStream* mOutputData;

	int DecompressDDSBC1();
	void WriteColorToBuffer(Vector color);
	void Release();
public:    
	ImageConverter(BufferInputStream* data);
    ImageConverter(const ImageConverter& src);
    ~ImageConverter();
	
	const BufferInputStream* GetOriginData() const;
	BufferOutputStream* GetOutputData() const;
    //const size_t GetOriginDataSize();
    //const size_t GetOutputDataSize();
    
	int SetOriginData(BufferInputStream* data);
    //int SetOriginDataSize(size_t dataSize);
    
    int CompressDDS();
	int DecompressDDS(ImageFormat &ddsFormat);
    int Convert(ImageFormat destFormat);
};

} // namespace Common
} // namespace NFE