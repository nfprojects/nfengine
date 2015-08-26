#include "PCH.hpp"
#include "../nfCommon/Image/Image.hpp"
#include "../nfCommon/InputStream.hpp"
#include "Constants.hpp"

using namespace NFE::Common;

const std::string testImagesPath = "./nfEngine/TestResources/ImageSamples/";
const int textureWidth = 16;
const int textureHeight = 16;

const std::string textureDDS_BC1 = "textureBC1.dds";
const std::string textureDDS_BC2 = "textureBC2.dds";
const std::string textureDDS_BC3 = "textureBC3.dds";
const std::string textureDDS_BC4 = "textureBC4.dds";
const std::string textureDDS_BC5 = "textureBC5.dds";
const std::string textureDDS_BC6H = "textureBC6H.dds";
const std::string textureDDS_BC7 = "textureBC7.dds";
const std::string textureDDS_MM = "textureBC1_MM.dds";

const std::string textureJPG = "textureJPG.jpg";

const std::string textureBMP4 = "textureBMP4.bmp";
const std::string textureBMP8 = "textureBMP8.bmp";
const std::string textureBMP16ARGB = "textureBMP16ARGB.bmp";
const std::string textureBMP16XRGB = "textureBMP16XRGB.bmp";
const std::string textureBMP16RGB = "textureBMP16RGB.bmp";
const std::string textureBMP24 = "textureBMP24.bmp";
const std::string textureBMP32ARGB = "textureBMP32ARGB.bmp";
const std::string textureBMP32XRGB = "textureBMP32XRGB.bmp";

const std::string texturePNG_RGB = "texturePNG_RGB.png";
const std::string texturePNG_RGBA = "texturePNG_RGBA.png";
const std::string texturePNG_A = "texturePNG_A.png";

ImageFormat testDataFormat = ImageFormat::RGB_UByte;
const int testDataWidth = 4;
const int testDataHeight = 4;
const int testDataSize = testDataWidth * testDataHeight * 3;
unsigned char testData[testDataSize] = { 0, 0, 0,   0, 0, 0,   255, 0,   0, 255, 0,   0,
                                         0, 0, 0,   0, 0, 0,   255, 0,   0, 255, 0,   0,
                                         0, 0, 255, 0, 0, 255, 0,   255, 0, 0,   255, 0,
                                         0, 0, 255, 0, 0, 255, 0,   255, 0, 0,   255, 0 };
const int testDataMaxMipmapNum = 2;

// Image format tables
const std::vector<ImageFormat> supportedConversionFormatsNonBC = { ImageFormat::RGB_UByte,
                                                                   ImageFormat::RGBA_UByte,
                                                                   ImageFormat::RGBA_Float,
                                                                   ImageFormat::A_UByte,
                                                                   ImageFormat::R_UByte,
                                                                   ImageFormat::R_Float };
// TODO Add BC4-BC7 format, when their conversion is supported
const std::vector<ImageFormat> supportedConversionFormatsBC = { ImageFormat::BC1, ImageFormat::BC2,
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
        //Load image
        ASSERT_TRUE(mImage->Load(mTexture.get()));

        //Convert to nonBC format and check texels
        for (auto i : supportedConversionFormatsNonBC)
        {
            Image imageToConvert(*mImage.get());

            ASSERT_TRUE(imageToConvert.Convert(i));
            ASSERT_EQ(i, imageToConvert.GetFormat());

            CheckTexels(&imageToConvert);
        }

        // Convert to BC, then to RGBA and check texels
        for (auto i : supportedConversionFormatsBC)
        {
            Image imageToConvert(*mImage.get());

            ASSERT_TRUE(imageToConvert.Convert(i));
            ASSERT_EQ(i, imageToConvert.GetFormat());

            ASSERT_TRUE(imageToConvert.Convert(testDataFormat));
            ASSERT_EQ(testDataFormat, imageToConvert.GetFormat());

            CheckTexels(&imageToConvert);
        }
    }

    // Function to compare texels of loaded images with sample image
    void CheckTexels(Image* img)
    {
        // Declare variables needed for sample image creation
        const int testSquareSize = 16 * 16 * 3;
        const int testSquareLineSize = 16 * 3;
        const int testSquareHalfLineSize = testSquareLineSize / 2;

        uchar testSquare[testSquareSize];
        ImageFormat textureFormat = img->GetFormat();

        int height = 16;
        int width = 16;

        // Building array, that image should be read as. It's different depending on the pixel format.
        for (int i = 0; i < height/2; i ++)
        {
            for (int j = 0; j < width/2; j++)
            {
                int index = (i * width + j) * 3;
                // 1st half
                testSquare[index] = 0;
                testSquare[index + 1] = 0;
                testSquare[index + 2] = 0;
                if (textureFormat == ImageFormat::A_UByte)
                {
                    testSquare[index] = 255;
                    testSquare[index + 1] = 255;
                    testSquare[index + 2] = 255;
                }
                else
                {
                    testSquare[index] = 0;
                    testSquare[index + 1] = 0;
                    testSquare[index + 2] = 0;
                }


                testSquare[testSquareHalfLineSize + index] = 255;
                if (textureFormat == ImageFormat::A_UByte)
                {
                    testSquare[testSquareHalfLineSize + index + 1] = 255;
                    testSquare[testSquareHalfLineSize + index + 2] = 255;
                }
                else
                {
                    testSquare[testSquareHalfLineSize + index + 1] = 0;
                    testSquare[testSquareHalfLineSize + index + 2] = 0;
                }

                // 2nd half
                index += testSquareSize / 2;
                if (textureFormat == ImageFormat::A_UByte)
                {
                    testSquare[index] = 255;
                    testSquare[index + 1] = 255;
                    testSquare[index + 2] = 255;
                }
                else
                {
                    testSquare[index] = 0;
                    testSquare[index + 1] = 0;
                    if (textureFormat == ImageFormat::R_UByte
                        || textureFormat == ImageFormat::R_Float)
                        testSquare[index + 2] = 0;
                    else
                        testSquare[index + 2] = 255;
                }

                if (textureFormat == ImageFormat::A_UByte)
                {
                    testSquare[testSquareHalfLineSize + index] = 255;
                    testSquare[testSquareHalfLineSize + index + 1] = 255;
                    testSquare[testSquareHalfLineSize + index + 2] = 255;
                }
                else
                {
                    testSquare[testSquareHalfLineSize + index] = 0;
                    if (textureFormat == ImageFormat::R_UByte
                        || textureFormat == ImageFormat::R_Float)
                        testSquare[testSquareHalfLineSize + index + 1] = 0;
                    else
                        testSquare[testSquareHalfLineSize + index + 1] = 255;
                    testSquare[testSquareHalfLineSize + index + 2] = 0;
                }
            }
        }

        // Texel size is always 3 in sample image, for compatibility reasons.
        const int singleTexelSize = 3;

        // Texel checking loop
        for (int i = 0; i < textureHeight; i++)
        {
            for (int j = 0; j < textureWidth; j++)
            {
                // Getting vector object and multiplying it by 255, because it's normalised to 1
                Color texel = img->GetMipmap()->GetTexel(j, i, textureFormat);
                texel *= 255.0f;
                uchar texelUCh[4];

                // Store vector in uchar table and remove compression errors
                VectorStoreUChar4(texel, texelUCh);
                RemoveCompression(texelUCh);

                // Get single texel from sample image
                uchar* testTexel = &testSquare[singleTexelSize * (i * textureWidth + j)];

                // Compare single texel of loaded image and sample image
                ASSERT_EQ(0, memcmp(testTexel, texelUCh, singleTexelSize * sizeof(uchar)));
            }
        }
    }

    // Function for removing compression errors
    void RemoveCompression(uchar* table)
    {
        for (int i = 0; i < 3; i++)
            table[i] = table[i] < 5 ? 0 : (table[i] > 250 ? 255 : table[i]);
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
    EXPECT_TRUE(mImage->SetData(testData, testDataWidth,
                                testDataHeight, testDataFormat));

    // After setting data, mImage should differ from uninitialized Image object
    EXPECT_NE(mImage->GetMipmapsNum(), imageEmpty.GetMipmapsNum());
    EXPECT_NE(mImage->GetData() == nullptr, imageEmpty.GetData() == nullptr);
    EXPECT_NE(mImage->GetFormat(), imageEmpty.GetFormat());
    EXPECT_NE(mImage->GetHeight(), imageEmpty.GetHeight());
    EXPECT_NE(mImage->GetWidth(), imageEmpty.GetWidth());

    Image imageCopy(*mImage.get());
    const uchar* mImageData = static_cast<const uchar*>(mImage->GetData());
    const uchar* imageCopyData = static_cast<const uchar*>(imageCopy.GetData());

    // After copying Image object, both objects should return the same data
    ASSERT_EQ(mImage->GetMipmapsNum(), imageCopy.GetMipmapsNum());
    ASSERT_EQ(0, memcmp(mImageData, imageCopyData, testDataSize));
    ASSERT_EQ(mImage->GetFormat(), imageCopy.GetFormat());
    ASSERT_EQ(mImage->GetHeight(), imageCopy.GetHeight());
    ASSERT_EQ(mImage->GetWidth(), imageCopy.GetWidth());
}

TEST_F(ImageTest, CopyAssignment)
{
    EXPECT_FALSE(std::is_copy_assignable<Image>::value);
}

TEST_F(ImageTest, MoveConstructor)
{
    EXPECT_FALSE(std::is_move_constructible<Image>::value);
}

TEST_F(ImageTest, MoveAssignment)
{
    EXPECT_FALSE(std::is_move_assignable<Image>::value);
}

TEST_F(ImageTest, SetData)
{
    // Setting data and checking all set information
    EXPECT_TRUE(mImage->SetData(testData, testDataWidth,
                                testDataHeight, testDataFormat));
    const uchar* mImageData = static_cast<const uchar*>(mImage->GetData());

    ASSERT_EQ(1, mImage->GetMipmapsNum());
    ASSERT_EQ(0, memcmp(mImageData, testData, testDataSize));
    ASSERT_EQ(testDataFormat, mImage->GetFormat());
}

TEST_F(ImageTest, GenerateMipmaps)
{
    // No data
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, testDataMaxMipmapNum));

    // BC4, BC5, BC6H and BC7 not supported
    EXPECT_TRUE(mImage->SetData(testData, testDataWidth,
        testDataHeight, ImageFormat::BC4));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, testDataMaxMipmapNum));

    EXPECT_TRUE(mImage->SetData(testData, testDataWidth,
                                testDataHeight, ImageFormat::BC5));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, testDataMaxMipmapNum));

    EXPECT_TRUE(mImage->SetData(testData, testDataWidth,
                                testDataHeight, ImageFormat::BC6H));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, testDataMaxMipmapNum));

    EXPECT_TRUE(mImage->SetData(testData, testDataWidth,
                                testDataHeight, ImageFormat::BC7));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, testDataMaxMipmapNum));

    // Successfully loaded picture
    mImage->Release();
    mTexture.reset(new FileInputStream((testImagesPath + textureJPG).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    ASSERT_TRUE(mImage->GenerateMipmaps(MipmapFilter::Box, testDataMaxMipmapNum));
    ASSERT_EQ(testDataMaxMipmapNum + 1, mImage->GetMipmapsNum());
}

TEST_F(ImageTest, Release)
{
    EXPECT_TRUE(mImage->SetData(testData, testDataWidth,
                                testDataHeight, testDataFormat));
    EXPECT_TRUE(mImage->GenerateMipmaps(MipmapFilter::Box, testDataMaxMipmapNum));
    EXPECT_EQ(testDataMaxMipmapNum + 1, mImage->GetMipmapsNum());

    // After releasing Image, all data should be purged
    mImage->Release();

    ASSERT_TRUE(mImage->GetData() == nullptr);
    ASSERT_EQ(0, mImage->GetMipmapsNum());
    ASSERT_EQ(0, mImage->GetWidth());
    ASSERT_EQ(0, mImage->GetHeight());
    ASSERT_EQ(ImageFormat::Unknown, mImage->GetFormat());
}

TEST_F(ImageTest, LoadJPG)
{
    mTexture.reset(new FileInputStream((testImagesPath + textureJPG).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());
}

TEST_F(ImageTest, LoadPNG)
{
    // TODO Enable, when proper PNG support is implemented
    /*
    mTexture.reset(new FileInputStream((testImagesPath + texturePNG_RGB).data()));
    LoadAssert(ImageFormat::RGB_UByte);
    CheckTexels(mImage.get());
    */

    mTexture.reset(new FileInputStream((testImagesPath + texturePNG_RGBA).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    // TODO Enable, when proper PNG support is implemented
    /*
    mTexture.reset(new FileInputStream((testImagesPath + texturePNG_A).data()));
    LoadAssert(ImageFormat::A_UByte);
    CheckTexels(mImage.get());
    */
}

TEST_F(ImageTest, LoadBMP)
{
    // 4bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP4).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    // 8bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP8).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    // 16bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP16ARGB).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    mTexture.reset(new FileInputStream((testImagesPath + textureBMP16XRGB).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    mTexture.reset(new FileInputStream((testImagesPath + textureBMP16RGB).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    // 24bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP24).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    // 32bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP32ARGB).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());

    mTexture.reset(new FileInputStream((testImagesPath + textureBMP32XRGB).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    CheckTexels(mImage.get());
}

TEST_F(ImageTest, LoadDDS)
{
    // BC1
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC1).data()));
    LoadAssert(ImageFormat::BC1);

    // BC2
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC2).data()));
    LoadAssert(ImageFormat::BC2);

    // BC3
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC3).data()));
    LoadAssert(ImageFormat::BC3);

    // BC4
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC4).data()));
    LoadAssert(ImageFormat::BC4);

    // BC5
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC5).data()));
    LoadAssert(ImageFormat::BC5);

    /* TODO Enable, when BC6H and BC7 support is implemented
    // BC6H
    mTexture.reset(new FileInputStream(textureDDS_BC6H.data()));
    LoadAssert(ImageFormat::BC6H);

    // BC7
    mTexture.reset(new FileInputStream(textureDDS_BC7.data()));
    LoadAssert(ImageFormat::BC7);
    */

    // DDS format with mipmaps
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_MM).data()));
    LoadAssert(ImageFormat::BC1);
    ASSERT_EQ(5, mImage->GetMipmapsNum());
}

TEST_F(ImageTest, ImageFormatError)
{
    // Cast to ImageFormat some dummy value, that does not occur in this enum
    ImageFormat dummy = static_cast<ImageFormat>(testDataSize);
    // It should result in an unknown format
    ASSERT_EQ(BitsPerPixel(ImageFormat::Unknown), BitsPerPixel(dummy));
}

TEST_F(ImageTest, ConvertJPG)
{
    mTexture.reset(new FileInputStream((testImagesPath + textureJPG).data()));
    ConvertAssert();
}

TEST_F(ImageTest, ConvertPNG)
{
    // TODO Enable, when proper PNG support is implemented
    //mTexture.reset(new FileInputStream((testImagesPath + texturePNG_RGB).data()));
    //ConvertAssert();

    mTexture.reset(new FileInputStream((testImagesPath + texturePNG_RGBA).data()));
    ConvertAssert();

    // TODO Enable, when proper PNG support is implemented
    //mTexture.reset(new FileInputStream((testImagesPath + texturePNG_A).data()));
    //ConvertAssert();
}

TEST_F(ImageTest, ConvertBMP)
{
    /* TODO Enable, when 16bit BMP support is implemented
    // 16bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP16ARGB).data()));
    ConvertAssert();

    mTexture.reset(new FileInputStream((testImagesPath + textureBMP16XRGB).data()));
    ConvertAssert();

    mTexture.reset(new FileInputStream((testImagesPath + textureBMP16RGB).data()));
    ConvertAssert();
    */

    // 24bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP24).data()));
    ConvertAssert();

    /* TODO Enable, when 32bit BMP support is implemented
    // 32bpp
    mTexture.reset(new FileInputStream((testImagesPath + textureBMP32ARGB).data()));
    ConvertAssert();

    mTexture.reset(new FileInputStream((testImagesPath + textureBMP32XRGB).data()));
    ConvertAssert();
    */
}

TEST_F(ImageTest, ConvertDDS)
{
    // BC1
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC1).data()));
    ConvertAssert();

    // BC2
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC2).data()));
    ConvertAssert();

    // BC3
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC3).data()));
    ConvertAssert();

    /* TODO Enable, when BC4 - BC7 support is implemented
    // BC4
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC4).data()));
    ConvertAssert();

    // BC5
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC5).data()));
    ConvertAssert();

    // BC6H
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC6H).data()));
    ConvertAssert();

    // BC7
    mTexture.reset(new FileInputStream((testImagesPath + textureDDS_BC7).data()));
    ConvertAssert();
    */
}

TEST_F(ImageTest, ConvertErrors)
{
    mTexture.reset(new FileInputStream((testImagesPath + textureJPG).data()));
    LoadAssert(ImageFormat::RGBA_UByte);

    // Conversion to unknown format
    ASSERT_FALSE(mImage->Convert(ImageFormat::Unknown));

    // Conversion with no data - nothing to convert
    mImage->Release();
    ASSERT_FALSE(mImage->Convert(ImageFormat::RGBA_UByte));
}