#include "PCH.hpp"
#include "../nfCommon/Image.hpp"
#include "../nfCommon/InputStream.hpp"
#include "Test.hpp"
//#include "libsquish/squish.h"

using namespace NFE::Common;

class ImageTest : public testing::Test
{
protected:
    std::unique_ptr<Image> mImage;

    void SetUp()
    {
        EXPECT_NO_THROW(mImage.reset(new Image()));
    }
};

TEST_F(ImageTest, BasicConstructor)
{
    // For uninitialized mImage nothing should work...
    ASSERT_EQ(0, mImage->GetWidth());
    ASSERT_EQ(0, mImage->GetHeight());
    ASSERT_EQ(0, mImage->GetMipmapsNum());
    ASSERT_TRUE(mImage->GetData() == nullptr);
    //ASSERT_TRUE(mImage->GetMipmap(0).data == nullptr);
    ASSERT_EQ(ImageFormat::Unknown, mImage->GetFormat());
}

TEST_F(ImageTest, CopyConstructor)
{
    Image imageCopy1;
	imageCopy1.SetData(const_cast<char*>(TEXT.data()), 5, 5, ImageFormat::BC1);
	Image imageCopy2(imageCopy1);

	ASSERT_EQ(1, imageCopy2.GetMipmapsNum());
	ASSERT_EQ(const_cast<char*>(TEXT.data()), imageCopy2.GetData());
	ASSERT_EQ(ImageFormat::BC1, imageCopy2.GetFormat());
}

TEST_F(ImageTest, SetData)
{
    // Setting dummy data and additional information
    mImage->SetData(const_cast<char*>(TEXT.data()), 5, 5, ImageFormat::BC1);
    ASSERT_EQ(1, mImage->GetMipmapsNum());
    ASSERT_EQ(const_cast<char*>(TEXT.data()), mImage->GetData());
    ASSERT_EQ(ImageFormat::BC1, mImage->GetFormat());
}

TEST_F(ImageTest, GenerateMipmaps)
{
    // No data
    ASSERT_EQ(1, mImage->GenerateMipmaps(5));

    // Unknown mImage format
    mImage->SetData(const_cast<char*>(TEXT.data()), 5, 5, ImageFormat::Unknown);
    ASSERT_EQ(1, mImage->GenerateMipmaps(5));

    // block-coding not supported
    mImage->SetData(const_cast<char*>(TEXT.data()), 5, 5, ImageFormat::BC1);
    ASSERT_EQ(1, mImage->GenerateMipmaps(5));

    // successfully loaded picture
    FileInputStream texture(textureJPG.data());
    ASSERT_EQ(0, mImage->Load(&texture));
    ASSERT_EQ(0, mImage->GenerateMipmaps(5));
    ASSERT_EQ(5, mImage->GetMipmapsNum());
}

TEST_F(ImageTest, Release)
{
    FileInputStream texture(textureJPG.data());
    EXPECT_EQ(0, mImage->Load(&texture));
    EXPECT_EQ(0, mImage->GenerateMipmaps(5));
    EXPECT_EQ(5, mImage->GetMipmapsNum());
    mImage->Release();
    ASSERT_EQ(0, mImage->GetMipmapsNum());
    ASSERT_EQ(0, mImage->GetWidth());
    ASSERT_EQ(0, mImage->GetHeight());
}

TEST_F(ImageTest, Load)
{
    FileInputStream* texture;
    // lambda to load and assert images
    auto LoadAssert = [&]()
    {
        ASSERT_EQ(0, mImage->Load(texture));
        ASSERT_EQ(240, mImage->GetWidth());
        ASSERT_EQ(212, mImage->GetHeight());
    };

    // Testing Load() for different types of mImage files
    // JPEG
    texture = new FileInputStream(textureJPG.data());
    LoadAssert();
    ASSERT_EQ(ImageFormat::RGBA_UByte, mImage->GetFormat());
    delete texture;

    // PNG
    texture = new FileInputStream(texturePNG_RGB.data());
    LoadAssert();
    ASSERT_TRUE(ImageFormat::RGBA_UByte == mImage->GetFormat() ||
                ImageFormat::A_UByte == mImage->GetFormat() ||
                ImageFormat::RGB_UByte == mImage->GetFormat());
    delete texture;

    // BMP
    texture = new FileInputStream(textureBMP.data());
    LoadAssert();
    ASSERT_EQ(ImageFormat::RGBA_UByte, mImage->GetFormat());
    delete texture;

    // DDS
    //   BC1
    texture = new FileInputStream(textureDDS_BC1.data());
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC1, mImage->GetFormat());
    delete texture;

    //   BC2
    texture = new FileInputStream(textureDDS_BC2.data());
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC2, mImage->GetFormat());
    delete texture;

    //   BC3
    texture = new FileInputStream(textureDDS_BC3.data());
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC3, mImage->GetFormat());
    delete texture;

    //   BC4
    texture = new FileInputStream(textureDDS_BC4.data());
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC4, mImage->GetFormat());
    delete texture;

    //   BC5
    texture = new FileInputStream(textureDDS_BC5.data());
    LoadAssert();
    ASSERT_EQ(ImageFormat::BC5, mImage->GetFormat());
    delete texture;
}

TEST_F(ImageTest, BitsPerPixel)
{
    ASSERT_EQ(8, Image::BitsPerPixel(ImageFormat::R_UByte));
    ASSERT_EQ(0, Image::BitsPerPixel(ImageFormat::Unknown));
}

TEST_F(ImageTest, Convert)
{
    FileInputStream* texture;
    // lambda to load, convert and assert images
    auto LoadAndConvertAssert = [&]()
    {
        ASSERT_EQ(0, mImage->Load(texture));

        ASSERT_EQ(0, mImage->Convert(ImageFormat::A_UByte));
        ASSERT_EQ(ImageFormat::A_UByte, mImage->GetFormat());

        ASSERT_EQ(0, mImage->Convert(ImageFormat::R_UByte));
        ASSERT_EQ(ImageFormat::R_UByte, mImage->GetFormat());

        ASSERT_EQ(0, mImage->Convert(ImageFormat::RGB_UByte));
        ASSERT_EQ(ImageFormat::RGB_UByte, mImage->GetFormat());

        ASSERT_EQ(0, mImage->Convert(ImageFormat::RGBA_UByte));
        ASSERT_EQ(ImageFormat::RGBA_UByte, mImage->GetFormat());

        ASSERT_EQ(0, mImage->Convert(ImageFormat::R_Float));
        ASSERT_EQ(ImageFormat::R_Float, mImage->GetFormat());

        ASSERT_EQ(0, mImage->Convert(ImageFormat::RGBA_Float));
        ASSERT_EQ(ImageFormat::RGBA_Float, mImage->GetFormat());
    };

    // Testing Convert() for different types of mImage files
    // JPEG
    texture = new FileInputStream(textureJPG.data());
    LoadAndConvertAssert();
    delete texture;

    // PNG
    texture = new FileInputStream(texturePNG_RGB.data());
    LoadAndConvertAssert();
    delete texture;

    // BMP
    texture = new FileInputStream(textureBMP.data());
    LoadAndConvertAssert();
    delete texture;

    // to Unknown format
    ASSERT_EQ(1, mImage->Convert(ImageFormat::Unknown));

    // DDS - currently no conversion for dds
    //   BC1
    texture = new FileInputStream(textureDDS_BC1.data());
    ASSERT_EQ(1, mImage->Convert(ImageFormat::RGBA_UByte));
    delete texture;

    //   BC2
    texture = new FileInputStream(textureDDS_BC2.data());
    ASSERT_EQ(1, mImage->Convert(ImageFormat::RGBA_UByte));
    delete texture;

    //   BC3
    texture = new FileInputStream(textureDDS_BC3.data());
    ASSERT_EQ(1, mImage->Convert(ImageFormat::RGBA_UByte));
    delete texture;

    //   BC4
    texture = new FileInputStream(textureDDS_BC4.data());
    ASSERT_EQ(1, mImage->Convert(ImageFormat::RGBA_UByte));
    delete texture;

    //   BC5
    texture = new FileInputStream(textureDDS_BC5.data());
    ASSERT_EQ(1, mImage->Convert(ImageFormat::RGBA_UByte));
    delete texture;

    // NO DATA - nothing to convert
    mImage->Release();
    ASSERT_EQ(0, mImage->Convert(ImageFormat::RGBA_UByte));

    // from Unknown format
    mImage->SetData(const_cast<char*>(TEXT.data()), 5, 5, ImageFormat::Unknown);
    ASSERT_EQ(1, mImage->Convert(ImageFormat::RGBA_UByte));
}

TEST_F(ImageTest, Testtest)
{
	FileInputStream* texture;
	auto Go = [&](std::string a)
	{
		mImage->Release();
		std::string path = testImagesPath + a;
		texture = new FileInputStream(path.data());
		int ret = mImage->Load(texture);
		std::cout << ret << " " << Image::FormatToStr(mImage->GetFormat()) << std::endl;
		delete texture;
	};
	Go(textureDDS_BC1);
    mImage->ConvDDS();
	mImage->Printme();
	/*Go(textureDDS_BC2);
	Go(textureDDS_BC3);
	Go(textureDDS_BC4);
	Go(textureDDS_BC5);
	Go(textureDDS_MM);
	Go(textureJPG);
	//mImage->Printme();
	
	Go(texturePNG_RGB);
	//mImage->Printme();
	Go(texturePNG_RGBA);
	//mImage->Printme();
	Go(texturePNG_A);
	//mImage->Printme();
	Go(textureBMP);
	//mImage->Printme();
    */

	/*if (ret != 0 || mImage->GetFormat() == ImageFormat::Unknown)
		std::cout << "load failed" << std::endl;
	else
		mImage->Printme();*/
}