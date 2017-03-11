#include "PCH.hpp"
#include "nfCommon/Image/Image.hpp"
#include "nfCommon/IO/InputStream.hpp"
#include "nfCommon/IO/OutputStream.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"
#include "Constants.hpp"

#include <memory>


using namespace NFE;
using namespace NFE::Common;

namespace {
const std::string TEST_IMAGES_PATH = "./nfEngine/TestResources/ImageSamples/";
const std::string TEST_IMAGES_SAVEPATH = "./nfEngine/TestResources/ImageSamples/SaveTests/";
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

const std::string TEXTURE_BMP4 = "textureBMP4.bmp";
const std::string TEXTURE_BMP8 = "textureBMP8.bmp";
const std::string TEXTURE_BMP16ARGB = "textureBMP16ARGB.bmp";
const std::string TEXTURE_BMP16XRGB = "textureBMP16XRGB.bmp";
const std::string TEXTURE_BMP16RGB = "textureBMP16RGB.bmp";
const std::string TEXTURE_BMP24 = "textureBMP24.bmp";
const std::string TEXTURE_BMP32ARGB = "textureBMP32ARGB.bmp";
const std::string TEXTURE_BMP32XRGB = "textureBMP32XRGB.bmp";

const std::string TEXTURE_PNG_RGB = "texturePNG_RGB.png";
const std::string TEXTURE_PNG_RGBA = "texturePNG_RGBA.png";
const std::string TEXTURE_PNG_RGBA_PALETTE = "texturePNG_RGBA_palette.png";
const std::string TEXTURE_PNG_RGBA_INTERLACED = "texturePNG_RGBA_interlaced.png";
const std::string TEXTURE_PNG_GA = "texturePNG_GA.png";

const int GRAYSCALE_R = 53;
const int GRAYSCALE_B = 17;
const int GRAYSCALE_G = 183;

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
} // namespace

class ImageTest : public testing::Test
{
protected:
    // Pointers to used structures
    std::unique_ptr<Image> mImage;
    std::unique_ptr<FileInputStream> mImageFile;

    std::unique_ptr<Image> mTestImageA;
    std::unique_ptr<Image> mTestImageR;
    std::unique_ptr<Image> mTestImageRGB;

    void SetUp()
    {
        mImage.reset(new Image());
    }

    static void SetUpTestCase()
    {
        FileSystem::CreateDirIfNotExist(TEST_IMAGES_SAVEPATH);
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

    void FillTestImage(ImageFormat textureFormat)
    {
        const int height = 16;
        const int width = 16;

        if (textureFormat == ImageFormat::A_UByte || textureFormat == ImageFormat::R_UByte)
        {
            const int testSquareSize = height * width;
            const int testSquareLineSize = width;
            const int testSquareHalfLineSize = testSquareLineSize / 2;

            uint8 testSquare[testSquareSize];


            for (int i = 0; i < height / 2; i++)
            {
                for (int j = 0; j < width / 2; j++)
                {
                    int index = (i * width + j);
                    // 1st half
                    if (textureFormat == ImageFormat::A_UByte)
                        testSquare[index] = 255;
                    else
                        testSquare[index] = 0;

                    testSquare[testSquareHalfLineSize + index] = 255;

                    // 2nd half
                    index += testSquareSize / 2;
                    if (textureFormat == ImageFormat::A_UByte)
                    {
                        testSquare[index] = 255;
                        testSquare[testSquareHalfLineSize + index] = 255;
                    } else
                    {
                        testSquare[index] = 0;
                        testSquare[testSquareHalfLineSize + index] = 0;
                    }
                }
            }

            if (textureFormat == ImageFormat::A_UByte)
            {
                mTestImageA.reset(new Image());
                mTestImageA->SetData(testSquare, width, height, textureFormat);
            }
            else
            {
                mTestImageR.reset(new Image());
                mTestImageR->SetData(testSquare, width, height, textureFormat);
            }

        }
        else
        {
            // Declare variables needed for sample image creation
            const int testSquareSize = height * width * 3;
            const int testSquareLineSize = width * 3;
            const int testSquareHalfLineSize = testSquareLineSize / 2;

            uint8 testSquare[testSquareSize];

            // Building array, that image should be read as. It's different depending on the pixel format.
            for (int i = 0; i < height / 2; i++)
            {
                for (int j = 0; j < width / 2; j++)
                {
                    int index = (i * width + j) * 3;
                    // 1st half
                    testSquare[index] = 0;
                    testSquare[index + 1] = 0;
                    testSquare[index + 2] = 0;

                    testSquare[testSquareHalfLineSize + index] = 255;
                    testSquare[testSquareHalfLineSize + index + 1] = 0;
                    testSquare[testSquareHalfLineSize + index + 2] = 0;

                    // 2nd half
                    index += testSquareSize / 2;

                    testSquare[index] = 0;
                    testSquare[index + 1] = 0;
                    testSquare[index + 2] = 255;

                    testSquare[testSquareHalfLineSize + index] = 0;
                    testSquare[testSquareHalfLineSize + index + 1] = 255;
                    testSquare[testSquareHalfLineSize + index + 2] = 0;
                }
            }

            mTestImageRGB.reset(new Image());
            mTestImageRGB->SetData(testSquare, width, height, textureFormat);
        }
    }

    // Function to compare texels of loaded images with sample image
    void CheckTexels(Image* img)
    {
        Image* imgPtr = nullptr;
        ImageFormat textureFormat = img->GetFormat();

        if (textureFormat == ImageFormat::A_UByte)
            imgPtr = mTestImageA.get();
        else if (textureFormat == ImageFormat::R_UByte
                 || textureFormat == ImageFormat::R_Float)
            imgPtr = mTestImageR.get();
        else
        {
            imgPtr = mTestImageRGB.get();
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
                Color testTexel = imgPtr->GetMipmap()->GetTexel(j, i, imgPtr->GetFormat());
                texel *= 255.0f;
                testTexel *= 255.0f;

                uint8 texelUCh[4];
                uint8 testTexelUCh[4];

                // Store vector in uint8 table and remove compression errors
                VectorStoreUChar4(texel, texelUCh);
                VectorStoreUChar4(testTexel, testTexelUCh);

                // Add scoped trace, to give information where exactly the error occured
                SCOPED_TRACE("X: " + std::to_string(j) + " Y: " + std::to_string(i));

                // Compare single texel of loaded image and sample image
                for (int m = 0; m < singleTexelSize; m++)
                {
                    uint8 diff = (testTexelUCh[m] > texelUCh[m] ?
                                    testTexelUCh[m] - texelUCh[m] :
                                    texelUCh[m] - testTexelUCh[m]);
                    ASSERT_LE(diff, COMPRESSION_ARTEFACT_TRESHOLD);
                }
            }
        }
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
    const uint8* mImageData = static_cast<const uint8*>(mImage->GetData());
    const uint8* imageCopyData = static_cast<const uint8*>(imageCopy.GetData());

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
    const uint8* mImageData = static_cast<const uint8*>(mImage->GetData());

    ASSERT_EQ(1, mImage->GetMipmapsNum());
    ASSERT_EQ(0, memcmp(mImageData, TEST_DATA, TEST_DATA_SIZE));
    ASSERT_EQ(TEST_DATA_FORMAT, mImage->GetFormat());
}

TEST_F(ImageTest, Grayscale)
{
    // Setting data and checking all set information
    EXPECT_TRUE(mImage->SetData(TEST_DATA, TEST_DATA_WIDTH,
                                TEST_DATA_HEIGHT, TEST_DATA_FORMAT));

    ASSERT_TRUE(mImage->Grayscale());

    ASSERT_EQ(1, mImage->GetMipmapsNum());
    ASSERT_EQ(TEST_DATA_FORMAT, mImage->GetFormat());

    const int singleTexelSize = 3;

    for (int i = 0; i < TEST_DATA_HEIGHT; i++)
    {
        for (int j = 0; j < TEST_DATA_WIDTH; j++)
        {
            // Getting vector object and multiplying it by 255, because it's normalised to 1
            Color texel = mImage->GetMipmap()->GetTexel(j, i, TEST_DATA_FORMAT);
            texel *= 255.0f;

            uint8 texelUCh[4];
            uint8 testTexelUCh[4];
            // Store vector in uint8 table
            VectorStoreUChar4(texel, texelUCh);

            if (i < 2)
            {
                if (j < 2)
                    testTexelUCh[0] = 0;
                else
                    testTexelUCh[0] = GRAYSCALE_R;
            } else
            {
                if (j < 2)
                    testTexelUCh[0] = GRAYSCALE_B;
                else
                    testTexelUCh[0] = GRAYSCALE_G;
            }

            testTexelUCh[1] = testTexelUCh[2] = testTexelUCh[0];
            testTexelUCh[3] = 255;

            // Add scoped trace, to give information where exactly the error occured
            SCOPED_TRACE("X: " + std::to_string(j) + " Y: " + std::to_string(i));

            // Compare single texel of loaded image and sample image
            ASSERT_EQ(0, memcmp(testTexelUCh, texelUCh, singleTexelSize * sizeof(uint8)));
        }
    }
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
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGB_UByte));
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
    FillTestImage(ImageFormat::RGB_UByte);
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_JPG).data()));
    LoadCheck(ImageFormat::RGB_UByte);
}

TEST_F(ImageTest, SaveJPG)
{
    FillTestImage(ImageFormat::RGB_UByte);
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_JPG).data()));
    LoadCheck(ImageFormat::RGB_UByte);

    {
        FileOutputStream outFile((TEST_IMAGES_SAVEPATH + TEXTURE_JPG + "_saved.jpg").data());
        ASSERT_FALSE(ImageType::GetImageType("JPG")->Save(mImage.get(), &outFile));
        ASSERT_TRUE(mImage->Convert(ImageFormat::RGBA_UByte));
        ASSERT_TRUE(ImageType::GetImageType("JPG")->Save(mImage.get(), &outFile));
    }

    mImageFile.reset(new FileInputStream((TEST_IMAGES_SAVEPATH + TEXTURE_JPG + "_saved.jpg").data()));
    LoadCheck(ImageFormat::RGB_UByte);
}

TEST_F(ImageTest, LoadPNG)
{
    FillTestImage(ImageFormat::RGB_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGB).data()));
    LoadCheck(ImageFormat::RGB_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA_PALETTE).data()));
    LoadCheck(ImageFormat::RGB_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA_INTERLACED).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    // When loading grayscale PNG, convert testImage just before checking texels
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_GA).data()));
    LoadAssert(ImageFormat::RGBA_UByte);
    ASSERT_TRUE(mTestImageRGB->Grayscale());
    CheckTexels(mImage.get());
}

TEST_F(ImageTest, LoadBMP)
{
    FillTestImage(ImageFormat::RGB_UByte);

    // 4bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP4).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    // 8bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP8).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    // 16bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16ARGB).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16XRGB).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16RGB).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    // 24bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP24).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    // 32bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP32ARGB).data()));
    LoadCheck(ImageFormat::RGBA_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP32XRGB).data()));
    LoadCheck(ImageFormat::RGBA_UByte);
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
    FillTestImage(ImageFormat::RGB_UByte);
    FillTestImage(ImageFormat::A_UByte);
    FillTestImage(ImageFormat::R_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_JPG).data()));
    ConvertAssert();
}

TEST_F(ImageTest, ConvertPNG)
{
    FillTestImage(ImageFormat::RGB_UByte);
    FillTestImage(ImageFormat::A_UByte);
    FillTestImage(ImageFormat::R_UByte);

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGB).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA_PALETTE).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_PNG_RGBA_INTERLACED).data()));
    ConvertAssert();
}

TEST_F(ImageTest, ConvertBMP)
{
    FillTestImage(ImageFormat::RGB_UByte);
    FillTestImage(ImageFormat::A_UByte);
    FillTestImage(ImageFormat::R_UByte);

    // 16bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16ARGB).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16XRGB).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP16RGB).data()));
    ConvertAssert();

    // 24bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP24).data()));
    ConvertAssert();

    // 32bpp
    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP32ARGB).data()));
    ConvertAssert();

    mImageFile.reset(new FileInputStream((TEST_IMAGES_PATH + TEXTURE_BMP32XRGB).data()));
    ConvertAssert();
}

TEST_F(ImageTest, ConvertDDS)
{
    FillTestImage(ImageFormat::RGB_UByte);
    FillTestImage(ImageFormat::A_UByte);
    FillTestImage(ImageFormat::R_UByte);

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
    ASSERT_NO_FATAL_FAILURE(LoadAssert(ImageFormat::RGB_UByte));

    // Conversion to unknown format
    ASSERT_FALSE(mImage->Convert(ImageFormat::Unknown));

    // Conversion with no data - nothing to convert
    mImage->Release();
    ASSERT_FALSE(mImage->Convert(ImageFormat::RGB_UByte));
}
