#include "PCH.hpp"
#include "Constants.hpp"

const std::string TEST_PACK_PATH = "testfile.nfp";
const std::string TEST_SAMPLE_FILE_DIR = "./TestDir/";
const std::string TEST_SAMPLE_FILE_PREFIX = "samplefile_";

const std::string TEXT = "The quick brown fox jumps over the lazy dog";
const size_t TEXTSIZE = TEXT.length();


const std::string testImagesPath = "./nfEngine/TestResources/ImageSamples/";
const int textureWidth = 16;
const int textureHeight = 16;

const std::string textureDDS_BC1    = "textureBC1.dds";
const std::string textureDDS_BC2    = "textureBC2.dds";
const std::string textureDDS_BC3    = "textureBC3.dds";
const std::string textureDDS_BC4    = "textureBC4.dds";
const std::string textureDDS_BC5    = "textureBC5.dds";
const std::string textureDDS_BC6H   = "textureBC6H.dds";
const std::string textureDDS_BC7    = "textureBC7.dds";
const std::string textureDDS_MM     = "textureBC1_MM.dds";

const std::string textureJPG        = "textureJPG.jpg";

const std::string textureBMP16ARGB  = "textureBMP16ARGB.bmp";
const std::string textureBMP16XRGB  = "textureBMP16XRGB.bmp";
const std::string textureBMP16RGB   = "textureBMP16RGB.bmp";
const std::string textureBMP24      = "textureBMP24.bmp";
const std::string textureBMP32ARGB  = "textureBMP32ARGB.bmp";
const std::string textureBMP32XRGB  = "textureBMP32XRGB.bmp";

const std::string texturePNG_RGB    = "texturePNG_RGB.png";
const std::string texturePNG_RGBA   = "texturePNG_RGBA.png";
const std::string texturePNG_A      = "texturePNG_A.png";

const int brbgTestSquareWidth = 4;
const int brbgTestSquareHeight = 4;
const int brbgTestSquareDataSize = brbgTestSquareWidth * brbgTestSquareHeight * 3;
unsigned char brbgTestSquare[brbgTestSquareDataSize] = { 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
                                                         1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
                                                         0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0,
                                                         0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 };
const int maxTestSquareMipmapNum = 2;