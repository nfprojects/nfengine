#include "PCH.hpp"
#include "../nfCommon/Image/Image.hpp"
#include "../nfCommon/InputStream.hpp"
#include "Constants.hpp"

/**
 * As stated by VC++ compiler dev here:
 * http://stackoverflow.com/a/34027257
 * it's a warning that is safe to disable.
 */
#if defined(WIN32)
#pragma warning( disable : 4592)
#endif // defined(WIN32)

using namespace NFE::Common;

namespace {
const std::string TEST_IMAGES_PATH = "./nfEngine/TestResources/ImageSamples/";
const int TEXTURE_WIDTH = 16;
const int TEXTURE_HEIGHT = 16;

const std::string TEXTURE_DDS_BC1 = "textureBC1.dds";
const std::string TEXTURE_DDS_BC2 = "textureBC2.dds";
const std::string TEXTURE_DDS_BC3 = "textureBC3.dds";
const std::string TEXTURE_DDS_BC4 = "textureBC4.dds";
const std::string TEXTURE_DDS_BC5 = "textureBC5.dds";
const std::string TEXTURE_DDS_BC6H = "textureBC6H.dds";
const std::string TEXTURE_DDS_BC7 = "textureBC7.dds";
const std::string TEXTURE_DDS_MM = "textureBC1_MM.dds";

const std::string TEXTURE_JPG = "textureJPG.jpg";

const std::string TEXTURE_BMP16ARGB = "textureBMP16ARGB.bmp";
const std::string TEXTURE_BMP16XRGB = "textureBMP16XRGB.bmp";
const std::string TEXTURE_BMP16RGB = "textureBMP16RGB.bmp";
const std::string TEXTURE_BMP24 = "textureBMP24.bmp";
const std::string TEXTURE_BMP32ARGB = "textureBMP32ARGB.bmp";
const std::string TEXTURE_BMP32XRGB = "textureBMP32XRGB.bmp";

const std::string TEXTURE_PNG_RGB = "texturePNG_RGB.png";
const std::string TEXTURE_PNG_RGBA = "texturePNG_RGBA.png";
const std::string TEXTURE_PNG_A = "texturePNG_A.png";

ImageFormat TEST_DATA_FORMAT = ImageFormat::RGB_UByte;
const int TEST_DATA_WIDTH = 4;
const int TEST_DATA_HEIGHT = 4;
const int TEST_DATA_SIZE = TEST_DATA_WIDTH * TEST_DATA_HEIGHT * 3;
unsigned char TEST_DATA[TEST_DATA_SIZE] = { 0, 0,   0, 0, 0,   0, 255,   0, 0, 255,   0, 0,
                                            0, 0,   0, 0, 0,   0, 255,   0, 0, 255,   0, 0,
                                            0, 0, 255, 0, 0, 255,   0, 255, 0,   0, 255, 0,
                                            0, 0, 255, 0, 0, 255,   0, 255, 0,   0, 255, 0 };
const int TEST_DATA_MAX_MIPMAP_NUM = 2;
const int COMPRESSION_ARTEFACT_TRESHOLD = 20;

// Image format tables
const std::vector<ImageFormat> SUPPORTED_CONVERSION_FORMATS_NON_BC = { ImageFormat::RGB_UByte,
                                                                        ImageFormat::RGBA_UByte,
                                                                        ImageFormat::RGBA_Float,
                                                                        ImageFormat::A_UByte,
                                                                        ImageFormat::R_UByte,
                                                                        ImageFormat::R_Float };
// TODO Add BC4-BC7 format, when their conversion is supported
const std::vector<ImageFormat> SUPPORTED_CONVERSION_FORMATS_BC = { ImageFormat::BC1,
                                                                    ImageFormat::BC2,
                                                                    ImageFormat::BC3 };
}

class ImageTest : public testing::Test
{
protected:
    // Pointers to used structures
    std::unique_ptr<Image> mImage;
    std::unique_ptr<FileInputStream> mImageFile;

    void SetUp()
    {
        EXPECT_NO_THROW(mImage.reset(new Image()));
    }

    // Function to check Load() functionality
    void LoadAssert(ImageFormat fmt)
    {
        // Add scoped trace, to give information where exactly the error occured
        SCOPED_TRACE("Loading " + std::string(FormatToStr(fmt)));

        ASSERT_TRUE(mImage->Load(mImageFile.get()));
        ASSERT_EQ(fmt, mImage->GetFormat());
        ASSERT_EQ(TEXTURE_WIDTH, mImage->GetWidth());
        ASSERT_EQ(TEXTURE_HEIGHT, mImage->GetHeight());
    }

    void LoadCheck(ImageFormat fmt)
    {
        ASSERT_NO_FATAL_FAILURE(LoadAssert(fmt));
        CheckTexels(mImage.get());
    }
    // Function to check Convert() functionality
    void ConvertAssert()
    {
        // Load image
        ASSERT_TRUE(mImage->Load(mImageFile.get()));

        // Convert to nonBC format and check texels
        for (auto i : SUPPORTED_CONVERSION_FORMATS_NON_BC)
        {
            // Add scoped trace, to give information where exactly the error occured
            SCOPED_TRACE("Conversion to " + std::string(FormatToStr(i)));

            Image imageToConvert(*mImage.get());

            ASSERT_TRUE(imageToConvert.Convert(i));
            ASSERT_EQ(i, imageToConvert.GetFormat());

            CheckTexels(&imageToConvert);
        }

        // Convert to BC, then to RGBA and check texels
        for (auto i : SUPPORTED_CONVERSION_FORMATS_BC)
        {
            // Add scoped trace, to give information where exactly the error occured
            SCOPED_TRACE("Conversion to " + std::string(FormatToStr(i)));

            Image imageToConvert(*mImage.get());

            ASSERT_TRUE(imageToConvert.Convert(i));
            ASSERT_EQ(i, imageToConvert.GetFormat());

            ASSERT_TRUE(imageToConvert.Convert(TEST_DATA_FORMAT));
            ASSERT_EQ(TEST_DATA_FORMAT, imageToConvert.GetFormat());

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
        for (int i = 0; i < height / 2; i ++)
        {
            for (int j = 0; j < width / 2; j++)
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
        for (int i = 0; i < TEXTURE_HEIGHT; i++)
        {
            for (int j = 0; j < TEXTURE_WIDTH; j++)
            {
                // Getting vector object and multiplying it by 255, because it's normalised to 1
                Color texel = img->GetMipmap()->GetTexel(j, i, textureFormat);
                texel *= 255.0f;
                uchar texelUCh[4];

                // Store vector in uchar table and remove compression errors
                VectorStoreUChar4(texel, texelUCh);
                RemoveCompression(texelUCh);

                // Get single texel from sample image
                uchar* testTexel = &testSquare[singleTexelSize * (i * TEXTURE_WIDTH + j)];

                // Add scoped trace, to give information where exactly the error occured
                SCOPED_TRACE("X: " + std::to_string(j) + " Y: " + std::to_string(i));

                // Compare single texel of loaded image and sample image
                ASSERT_EQ(0, memcmp(testTexel, texelUCh, singleTexelSize * sizeof(uchar)));
            }
        }
    }

    // Function for removing compression artefacts
    void RemoveCompression(uchar* table)
    {
        int upperLimit = 255 - COMPRESSION_ARTEFACT_TRESHOLD;
        int lowerLimit = COMPRESSION_ARTEFACT_TRESHOLD;

        for (int i = 0; i < 3; i++)
            table[i] = table[i] < lowerLimit ? 0 : (table[i] > upperLimit ? 255 : table[i]);
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
    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, TEST_DATA_FORMAT));

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
    ASSERT_EQ(0, memcmp(mImageData, imageCopyData, TEST_DATA_SIZE));
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
    // Setting data and checking all set information
    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, TEST_DATA_FORMAT));
    const uchar* mImageData = static_cast<const uchar*>(mImage->GetData());

    ASSERT_EQ(1, mImage->GetMipmapsNum());
    ASSERT_EQ(0, memcmp(mImageData, TEST_DATA, TEST_DATA_SIZE));
    ASSERT_EQ(TEST_DATA_FORMAT, mImage->GetFormat());
}

TEST_F(ImageTest, GenerateMipmaps)
{
    // No data
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, TEST_DATA_MAX_MIPMAP_NUM));

    // BC4, BC5, BC6H and BC7 not supported
    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, ImageFormat::BC4));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, TEST_DATA_MAX_MIPMAP_NUM));

    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, ImageFormat::BC5));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, TEST_DATA_MAX_MIPMAP_NUM));

    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, ImageFormat::BC6H));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, TEST_DATA_MAX_MIPMAP_NUM));

    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, ImageFormat::BC7));
    ASSERT_FALSE(mImage->GenerateMipmaps(MipmapFilter::Box, TEST_DATA_MAX_MIPMAP_NUM));

    // Successfully loaded picture
    mImage->Release();
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_JPG).data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGBA_UByte));
    ASSERT_TRUE(mImage->GenerateMipmaps(MipmapFilter::Box, TEST_DATA_MAX_MIPMAP_NUM));
    ASSERT_EQ(TEST_DATA_MAX_MIPMAP_NUM + 1, mImage->GetMipmapsNum());
}

TEST_F(ImageTest, Release)
{
    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, TEST_DATA_FORMAT));
    EXPECT_TRUE(mImage->GenerateMipmaps(MipmapFilter::Box, TEST_DATA_MAX_MIPMAP_NUM));
    EXPECT_EQ(TEST_DATA_MAX_MIPMAP_NUM + 1, mImage->GetMipmapsNum());

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
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_JPG).data()));
    LoadCheck(ImageFormat::RGBA_UByte);
}

TEST_F(ImageTest, LoadPNG)
{
    // TODO Enable, when proper PNG support is implemented
    /*
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGB).data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGB_UByte));
    CheckTexels(mImage.get());
    */

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    // TODO Enable, when proper PNG support is implemented
    /*
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_A).data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::A_UByte));
    CheckTexels(mImage.get());
    */
}

TEST_F(ImageTest, LoadBMP)
{
    /* TODO Enable, when 16bit BMP support is implemented
    // 16bpp
    mImageFile.reset(new FileInputStream(TEXTURE_BMP16ARGB.data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGBA_UByte));
    CheckTexels(mImage.get());

    mImageFile.reset(new FileInputStream(TEXTURE_BMP16XRGB.data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGBA_UByte));
    CheckTexels(mImage.get());

    mImageFile.reset(new FileInputStream(TEXTURE_BMP16RGB.data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGBA_UByte));
    CheckTexels(mImage.get());
    */

    // 24bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP24).data()));
    LoadCheck(ImageFormat::RGBA_UByte);


    /* TODO Enable, when 32bit BMP support is implemented
    // 32bpp
    mImageFile.reset(new FileInputStream(TEXTURE_BMP32ARGB.data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGBA_UByte));
    CheckTexels(mImage.get());

    mImageFile.reset(new FileInputStream(TEXTURE_BMP32XRGB.data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGBA_UByte));
    CheckTexels(mImage.get());
    */
}

TEST_F(ImageTest, LoadDDS)
{
    // BC1
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC1).data()));
    LoadAssert(ImageFormat::BC1);

    // BC2
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC2).data()));
    LoadAssert(ImageFormat::BC2);

    // BC3
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC3).data()));
    LoadAssert(ImageFormat::BC3);

    // BC4
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC4).data()));
    LoadAssert(ImageFormat::BC4);

    // BC5
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC5).data()));
    LoadAssert(ImageFormat::BC5);

    /* TODO Enable, when BC6H and BC7 support is implemented
    // BC6H
    mImageFile.reset(new FileInputStream(TEXTURE_DDS_BC6H.data()));
    LoadAssert(ImageFormat::BC6H);

    // BC7
    mImageFile.reset(new FileInputStream(TEXTURE_DDS_BC7.data()));
    LoadAssert(ImageFormat::BC7);
    */

    // DDS format with mipmaps
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_MM).data()));
    LoadAssert(ImageFormat::BC1);
    ASSERT_EQ(5, mImage->GetMipmapsNum());
}

TEST_F(ImageTest, ImageFormatError)
{
    // Cast to ImageFormat some dummy value, that does not occur in this enum
    ImageFormat dummy = static_cast<ImageFormat>(TEST_DATA_SIZE);
    // It should result in an unknown format
    ASSERT_EQ(BitsPerPixel(ImageFormat::Unknown), BitsPerPixel(dummy));
}

TEST_F(ImageTest, ConvertJPG)
{
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_JPG).data()));
    ConvertAssert();
}

TEST_F(ImageTest, ConvertPNG)
{
    // TODO Enable, when proper PNG support is implemented
    //mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGB).data()));
    //ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA).data()));
    ConvertAssert();

    // TODO Enable, when proper PNG support is implemented
    //mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_A).data()));
    //ConvertAssert();
}

TEST_F(ImageTest, ConvertBMP)
{
    /* TODO Enable, when 16bit BMP support is implemented
    // 16bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16ARGB).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16XRGB).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16RGB).data()));
    ConvertAssert();
    */

    // 24bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP24).data()));
    ConvertAssert();

    /* TODO Enable, when 32bit BMP support is implemented
    // 32bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP32ARGB).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP32XRGB).data()));
    ConvertAssert();
    */
}

TEST_F(ImageTest, ConvertDDS)
{
    // BC1
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC1).data()));
    ConvertAssert();

    // BC2
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC2).data()));
    ConvertAssert();

    // BC3
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC3).data()));
    ConvertAssert();

    /* TODO Enable, when BC4 - BC7 support is implemented
    // BC4
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC4).data()));
    ConvertAssert();

    // BC5
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC5).data()));
    ConvertAssert();

    // BC6H
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC6H).data()));
    ConvertAssert();

    // BC7
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_DDS_BC7).data()));
    ConvertAssert();
    */
}

TEST_F(ImageTest, ConvertErrors)
{
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_JPG).data()));
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGBA_UByte));

    // Conversion to unknown format
    ASSERT_FALSE(mImage->Convert(ImageFormat::Unknown));

    // Conversion with no data - nothing to convert
    mImage->Release();
    ASSERT_FALSE(mImage->Convert(ImageFormat::RGBA_UByte));
}