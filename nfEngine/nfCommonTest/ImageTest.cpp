#include "PCH.hpp"
#include "../nfCommon/Image/Image.hpp"
#include "../nfCommon/InputStream.hpp"
#include "Constants.hpp"

using namespace NFE::Common;

// Format of TestSquare table
ImageFormat brbgTestSquareFormat = ImageFormat::RGB_UByte;

// TODO Add BC4-BC7 format, when their conversion is supported
// Table of formats that we can convert
const std::vector<ImageFormat> supportedConversionFormats = { ImageFormat::A_UByte,   ImageFormat::R_UByte,
                                                              ImageFormat::RGB_UByte, ImageFormat::RGBA_UByte,
                                                              ImageFormat::R_Float,   ImageFormat::RGBA_Float,
                                                              ImageFormat::BC1,       ImageFormat::BC2,
                                                              ImageFormat::BC3 };

class ImageTest : public testing::Test
{
protected:
    // Pointers to used structures
    std::unique_ptr<Image> mImage;
    std::unique_ptr<FileInputStream> mTexture;

    void SetUp()
    {
        EXPECT_NO_THROW(mImage.reset(new Image()));
    }

    // Function to check Load() functionality
    void LoadAssert(ImageFormat fmt)
    {
        ASSERT_TRUE(mImage->Load(mTexture.get()));
        ASSERT_EQ(fmt, mImage->GetFormat());
        EXPECT_EQ(textureWidth, mImage->GetWidth());
        EXPECT_EQ(textureHeight, mImage->GetHeight());
    }

    // Function to check Convert() functionality
    void ConvertAssert()
    {
        ASSERT_TRUE(mImage->Load(mTexture.get()));
        for (auto i : supportedConversionFormats)
        {
            Image imageToConvert(*mImage.get());
            ASSERT_TRUE(imageToConvert.Convert(i));
            ASSERT_EQ(i, imageToConvert.GetFormat());
        }
    }

    // Function to compare corner texels of sample images and TestSquare
    void CheckTexels()
    {
        Color leftTopCol     = mImage->GetMipmap()->GetTexel(0,            0, mImage->GetFormat());
        Color rightTopCol    = mImage->GetMipmap()->GetTexel(textureWidth, 0, mImage->GetFormat());
        Color leftBottomCol  = mImage->GetMipmap()->GetTexel(0,            textureHeight, mImage->GetFormat());
        Color rightBottomCol = mImage->GetMipmap()->GetTexel(textureWidth, textureHeight, mImage->GetFormat());

        uchar leftTopColUch[4], leftBottomColUch[4], rightTopColUch[4], rightBottomColUch[4];
        VectorStoreUChar4(leftTopCol,     leftTopColUch);
        VectorStoreUChar4(rightTopCol,    rightTopColUch);
        VectorStoreUChar4(leftBottomCol,  leftBottomColUch);
        VectorStoreUChar4(rightBottomCol, rightBottomColUch);

        const int singleTexelSize = 3;
        uchar leftTopUch[singleTexelSize], rightTopUch[singleTexelSize],
            leftBottomUch[singleTexelSize], rightBottomUch[singleTexelSize];
        void* lTopMargin = brbgTestSquare;
        void* rTopMargin = brbgTestSquare + (brbgTestSquareWidth - singleTexelSize) * sizeof(uchar);
        void* lBotMargin = brbgTestSquare - brbgTestSquareWidth                     * sizeof(uchar);
        void* rBotMargin = brbgTestSquare - singleTexelSize                         * sizeof(uchar);

        memcpy(leftTopUch,     lTopMargin, singleTexelSize * sizeof(uchar));
        memcpy(rightTopUch,    rTopMargin, singleTexelSize * sizeof(uchar));
        memcpy(leftBottomUch,  lBotMargin, singleTexelSize * sizeof(uchar));
        memcpy(rightBottomUch, rBotMargin, singleTexelSize * sizeof(uchar));

        ASSERT_EQ(0, memcmp(leftTopUch,     leftTopColUch,     singleTexelSize * sizeof(uchar)));
        ASSERT_EQ(0, memcmp(rightTopUch,    rightTopColUch,    singleTexelSize * sizeof(uchar)));
        ASSERT_EQ(0, memcmp(leftBottomUch,  leftBottomColUch,  singleTexelSize * sizeof(uchar)));
        ASSERT_EQ(0, memcmp(rightBottomUch, rightBottomColUch, singleTexelSize * sizeof(uchar)));
    }
};

TEST_F(ImageTest, BasicConstructor)
{
    // For uninitialized mImage nothing should work
    ASSERT_EQ(0, mImage->GetWidth());
    ASSERT_EQ(0, mImage->GetHeight());
    ASSERT_EQ(0, mImage->GetMipmapsNum());
    ASSERT_TRUE(mImage->GetData() == nullptr);
    ASSERT_TRUE(mImage->GetMipmap() == nullptr);
    ASSERT_EQ(ImageFormat::Unknown, mImage->GetFormat());
}

TEST_F(ImageTest, CopyConstructor)
{
    EXPECT_TRUE(std::is_copy_constructible<Image>::value);

    Image imageEmpty;
    EXPECT_TRUE(mImage->SetData(brbgTestSquare, brbgTestSquareWidth,
                                brbgTestSquareHeight, brbgTestSquareFormat));

    // After setting data, mImage should differ from uninitialized Image object
    EXPECT_NE(mImage->GetMipmapsNum(), imageEmpty.GetMipmapsNum());
    EXPECT_FALSE(mImage->GetData() == nullptr);
    EXPECT_TRUE(imageEmpty.GetData() == nullptr);
    EXPECT_NE(mImage->GetFormat(), imageEmpty.GetFormat());
    EXPECT_NE(mImage->GetHeight(), imageEmpty.GetHeight());
    EXPECT_NE(mImage->GetWidth(), imageEmpty.GetWidth());
    
    Image imageCopy(*mImage.get());
    const uchar* mImageData = static_cast<const uchar*>(mImage->GetData());
    const uchar* imageCopyData = static_cast<const uchar*>(imageCopy.GetData());

    // After copying Image object, both objects should return the same data
    ASSERT_EQ(mImage->GetMipmapsNum(), imageCopy.GetMipmapsNum());
    ASSERT_EQ(0, memcmp(mImageData, imageCopyData, brbgTestSquareDataSize));
    ASSERT_EQ(mImage->GetFormat(), imageCopy.GetFormat());
    ASSERT_EQ(mImage->GetHeight(), imageCopy.GetHeight());
    ASSERT_EQ(mImage->GetWidth(), imageCopy.GetWidth());
}

TEST_F(ImageTest, CopyAssignment)
{
    ASSERT_FALSE(std::is_copy_assignable<Image>::value);
}

TEST_F(ImageTest, MoveConstructor)
{
    ASSERT_FALSE(std::is_move_constructible<Image>::value);
}

TEST_F(ImageTest, MoveAssignment)
{
    ASSERT_FALSE(std::is_move_assignable<Image>::value);
}

TEST_F(ImageTest, SetData)
{
    // Setting data and additional information
    EXPECT_TRUE(mImage->SetData(brbgTestSquare, brbgTestSquareWidth,
                brbgTestSquareHeight, brbgTestSquareFormat));
    const uchar* mImageData = static_cast<const uchar*>(mImage->GetData());

    ASSERT_EQ(1, mImage->GetMipmapsNum());
    ASSERT_EQ(0, memcmp(mImageData, brbgTestSquare, brbgTestSquareDataSize));
    ASSERT_EQ(brbgTestSquareFormat, mImage->GetFormat());
}

TEST_F(ImageTest, GenerateMipmaps)
{
    // No data
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, maxTestSquareMipmapNum));

    // BC4, BC5, BC6H and BC7 not supported
    EXPECT_TRUE(mImage->SetData(brbgTestSquare, brbgTestSquareWidth,
                brbgTestSquareHeight, ImageFormat::BC4));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, maxTestSquareMipmapNum));

    EXPECT_TRUE(mImage->SetData(brbgTestSquare, brbgTestSquareWidth,
                brbgTestSquareHeight, ImageFormat::BC5));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, maxTestSquareMipmapNum));

    EXPECT_TRUE(mImage->SetData(brbgTestSquare, brbgTestSquareWidth,
                brbgTestSquareHeight, ImageFormat::BC6H));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, maxTestSquareMipmapNum));
    
    EXPECT_TRUE(mImage->SetData(brbgTestSquare, brbgTestSquareWidth,
                brbgTestSquareHeight, ImageFormat::BC7));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, maxTestSquareMipmapNum));
    
    // successfully loaded picture
    mImage->Release();
    mTexture.reset(new FileInputStream((testImagesPath + textureJPG).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    std::cout << "1" << std::endl;
    ASSERT_TRUE(mImage->GenerateMipmaps(MipmapFilter::Box, maxTestSquareMipmapNum));
    std::cout << "1" << std::endl;
    ASSERT_EQ(maxTestSquareMipmapNum, mImage->GetMipmapsNum());
}

TEST_F(ImageTest, Release)
{
    EXPECT_TRUE(mImage->SetData(brbgTestSquare, brbgTestSquareWidth,
                brbgTestSquareHeight, brbgTestSquareFormat));
    EXPECT_TRUE(mImage->GenerateMipmaps(MipmapFilter::Box, maxTestSquareMipmapNum));
    EXPECT_EQ(maxTestSquareMipmapNum, mImage->GetMipmapsNum());

    // After releasing Image, all data should be purged
    mImage->Release();

    ASSERT_EQ(0, mImage->GetMipmapsNum());
    ASSERT_TRUE(mImage->GetData() == nullptr);
    ASSERT_EQ(0, mImage->GetWidth());
    ASSERT_EQ(0, mImage->GetHeight());
}

TEST_F(ImageTest, LoadJPG)
{
    // Testing Load() for different types of mImage files
    // JPEG
    mTexture.reset(new FileInputStream(textureJPG.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();
}

TEST_F(ImageTest, LoadPNG)
{
    // PNG
    mTexture.reset(new FileInputStream(texturePNG_RGB.data()));
    LoadAssert(ImageFormat::RGB_UByte);
    CheckTexels();

    mTexture.reset(new FileInputStream(texturePNG_RGBA.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();

    mTexture.reset(new FileInputStream(texturePNG_A.data()));
    LoadAssert(ImageFormat::A_UByte);
    // Its grayscale and our TestSquare is in colour
    //CheckTexels();
}

TEST_F(ImageTest, LoadBMP)
{
    /* TODO Enable, when 16bit BMP support is implemented
    // BMP 16bit
    mTexture.reset(new FileInputStream(textureBMP16ARGB.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();

    mTexture.reset(new FileInputStream(textureBMP16XRGB.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();

    mTexture.reset(new FileInputStream(textureBMP16RGB.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();
    */

    // BMP 24bit
    mTexture.reset(new FileInputStream(textureBMP24.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();

    /* TODO Enable, when 32bit BMP support is implemented
    // BMP 32bit
    mTexture.reset(new FileInputStream(textureBMP32ARGB.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();

    mTexture.reset(new FileInputStream(textureBMP32XRGB.data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels();
    */
}

TEST_F(ImageTest, LoadDDS)
{
    // DDS
    //   BC1
    mTexture.reset(new FileInputStream(textureDDS_BC1.data()));
    LoadAssert(ImageFormat::BC1);

    //   BC2
    mTexture.reset(new FileInputStream(textureDDS_BC2.data()));
    LoadAssert(ImageFormat::BC2);

    //   BC3
    mTexture.reset(new FileInputStream(textureDDS_BC3.data()));
    LoadAssert(ImageFormat::BC3);
    
    //   BC4
    mTexture.reset(new FileInputStream(textureDDS_BC4.data()));
    LoadAssert(ImageFormat::BC4);

    //   BC5
    mTexture.reset(new FileInputStream(textureDDS_BC5.data()));
    LoadAssert(ImageFormat::BC5);

    /* TODO Enable, when BC6H and BC7 support is implemented
    //   BC6H
    mTexture.reset(new FileInputStream(textureDDS_BC6H.data()));
    LoadAssert(ImageFormat::BC6H);

    //   BC7
    mTexture.reset(new FileInputStream(textureDDS_BC7.data()));
    LoadAssert(ImageFormat::BC7);
    */
}

TEST_F(ImageTest, BitsPerPixel)
{
    ImageFormat dummy = static_cast<ImageFormat>(brbgTestSquareDataSize);
    ASSERT_EQ(BitsPerPixel(ImageFormat::Unknown), BitsPerPixel(dummy));
}

TEST_F(ImageTest, ConvertJPG)
{
    // Testing Load() for different types of mImage files
    // JPEG
    mTexture.reset(new FileInputStream(textureJPG.data()));
    ConvertAssert();
}

TEST_F(ImageTest, ConvertPNG)
{
    // PNG
    mTexture.reset(new FileInputStream(texturePNG_RGB.data()));
    ConvertAssert();

    mTexture.reset(new FileInputStream(texturePNG_RGBA.data()));
    ConvertAssert();

    mTexture.reset(new FileInputStream(texturePNG_A.data()));
    ConvertAssert();
}

TEST_F(ImageTest, ConvertBMP)
{
    /* TODO Enable, when 16bit BMP support is implemented
    // BMP 16bit
    mTexture.reset(new FileInputStream(textureBMP16ARGB.data()));
    ConvertAssert();
    
    mTexture.reset(new FileInputStream(textureBMP16XRGB.data()));
    ConvertAssert();
    
    mTexture.reset(new FileInputStream(textureBMP16RGB.data()));
    ConvertAssert();
    */

    // BMP 24bit
    mTexture.reset(new FileInputStream(textureBMP24.data()));
    ConvertAssert();

    /* TODO Enable, when 32bit BMP support is implemented
    // BMP 32bit
    mTexture.reset(new FileInputStream(textureBMP32ARGB.data()));
    ConvertAssert();

    mTexture.reset(new FileInputStream(textureBMP32XRGB.data()));
    ConvertAssert();
    */
}

TEST_F(ImageTest, ConvertDDS)
{
    // DDS
    //   BC1
    mTexture.reset(new FileInputStream(textureDDS_BC1.data()));
    ConvertAssert();

    //   BC2
    mTexture.reset(new FileInputStream(textureDDS_BC2.data()));
    ConvertAssert();

    //   BC3
    mTexture.reset(new FileInputStream(textureDDS_BC3.data()));
    ConvertAssert();

    /* TODO Enable, when BC4 - BC7 support is implemented
    //   BC4
    mTexture.reset(new FileInputStream(textureDDS_BC4.data()));
    ConvertAssert();

    //   BC5
    mTexture.reset(new FileInputStream(textureDDS_BC5.data()));
    ConvertAssert();

    //   BC6H
    mTexture.reset(new FileInputStream(textureDDS_BC6H.data()));
    ConvertAssert();

    //   BC7
    mTexture.reset(new FileInputStream(textureDDS_BC7.data()));
    ConvertAssert();
    */
}

TEST_F(ImageTest, ConvertErrors)
{
    mTexture.reset(new FileInputStream(textureJPG.data()));
    LoadAssert(ImageFormat::RGBA_UByte);

    // to Unknown format
    ASSERT_FALSE(mImage->Convert(ImageFormat::Unknown));

    // NO DATA - nothing to convert
    mImage->Release();
    ASSERT_FALSE(mImage->Convert(ImageFormat::RGBA_UByte));
}