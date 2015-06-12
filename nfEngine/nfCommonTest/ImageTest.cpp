#include "PCH.hpp"
#include "../nfCommon/Image.hpp"
#include "../nfCommon/InputStream.hpp"

using namespace NFE::Common;

const std::string textureDDS_BC1 = "../TestResources/ImageSamples/textureBC1.dds";
const std::string textureDDS_BC2 = "../TestResources/ImageSamples/textureBC2.dds";
const std::string textureDDS_BC3 = "../TestResources/ImageSamples/textureBC3.dds";
const std::string textureDDS_BC4 = "../TestResources/ImageSamples/textureBC4.dds";
const std::string textureDDS_BC5 = "../TestResources/ImageSamples/textureBC5.dds";
const std::string textureJPG = "../TestResources/ImageSamples/textureJPG.jpg";
const std::string textureBMP = "../TestResources/ImageSamples/textureBMP.bmp";
const std::string texturePNG = "../TestResources/ImageSamples/texturePNG.png";


TEST(ImageTest, BasicConstructor)
{
    Image image();
    
    // For uninitialized image nothing should work...
    ASSERT_EQ(0, image.GetWidth());
    ASSERT_EQ(0, image.GetHeight());
    ASSERT_EQ(0, image.GetMipmapsNum());
    ASSERT_TRUE(image.GetData() == nullptr);
    ASSERT_TRUE(image.GetMipmap(0) == nullptr);
    ASSERT_EQ(ImageFormat::Unknown, image.GetFormat());
}

TEST(ImageTest, SetData)
{
    Image image();
    std::string dataStr = "testString";
    
    // Setting dummy data and additional information
    image.SetData(dataStr.data(), 5, 5, ImageFormat::BC1);
    ASSERT_EQ(1, image.GetMipmapsNum());
    ASSERT_EQ(dataStr.data(), image.GetData());
    ASSERT_EQ(ImageFormat::BC1, image.GetFormat());
}

TEST(ImageTest, GenerateMipmaps)
{
    Image image();
    std::string dataStr = "testString";
    
    // No data
    ASSERT_EQ(1, image.GenerateMipmaps(5));
    
    // Unknown image format
    image.SetData(dataStr.data(), 5, 5, ImageFormat::Unknown);
    ASSERT_EQ(1, image.GenerateMipmaps(5));
    
    // block-coding not supported
    image.SetData(dataStr.data(), 5, 5, ImageFormat::BC1);
    ASSERT_EQ(1, image.GenerateMipmaps(5));
    
    // successfully loaded picture
    FileInputStream texture(textureJPG);
    ASSERT_EQ(0, image.Load(&texture));
    ASSERT_EQ(0, image.GenerateMipmaps(5));
    ASSERT_EQ(5, image.GetMipmapsNum());
}

TEST(ImageTest, Load)
{
    Image image();
    FileInputStream* texture;
    // lambda to load and assert images 
    auto LoadAssert = [&]()
    {
        ASSERT_EQ(0, image.Load(&texture));
        ASSERT_EQ(240, image.GetWidth());
        ASSERT_EQ(212, image.GetHeight());
    };
    
    // Testing Load() for different types of image files
    // JPEG
    texture = FileInputStream(textureJPG);
    LoadAssert();
    ASSERT_EQ(ImageFormat::RGBA_UByte, image.GetFormat())
    
    // PNG
    texture = FileInputStream(texturePNG);
    LoadAssert();
    ASSERT_TRUE(ImageFormat::RGBA_UByte == image.GetFormat() ||
                ImageFormat::A_UByte == image.GetFormat() ||
                ImageFormat::RGB_UByte == image.GetFormat());
    
    // BMP
    texture = FileInputStream(textureBMP);
    LoadAssert();
    ASSERT_EQ(ImageFormat::RGBA_UByte, image.GetFormat())
    
    // DDS
    //   BC1
    texture = FileInputStream(textureDDS_BC1);
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC1, image.GetFormat())
    
    //   BC2
    texture = FileInputStream(textureDDS_BC2);
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC2, image.GetFormat())
    
    //   BC3
    texture = FileInputStream(textureDDS_BC3);
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC3, image.GetFormat())
    
    //   BC4
    texture = FileInputStream(textureDDS_BC4);
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC4, image.GetFormat())
    
    //   BC5
    texture = FileInputStream(textureDDS_BC5);
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC5, image.GetFormat())
    
}

TEST(ImageTest, FormatToString)
{
    ASSERT_EQ(0, Image::FormatToString(ImageFormat::R_UByte).compare("R_uchar"));
    ASSERT_EQ(0, Image::FormatToString(ImageFormat::Unknown).compare("Unknown"));
}

TEST(ImageTest, BitsPerPixel)
{
    ASSERT_EQ(8, Image::BitsPerPixel(ImageFormat::R_UByte);
    ASSERT_EQ(0, Image::BitsPerPixel(ImageFormat::Unknown);
}